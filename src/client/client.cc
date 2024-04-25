#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/image.h>
#include <webcface/robot_model.h>
#include <webcface/canvas3d.h>
#include <webcface/canvas2d.h>
#include <webcface/common/def.h>
#include <string>
#include <chrono>
#include <algorithm>
#include <memory>
#include "../message/message.h"
#include "client_internal.h"

WEBCFACE_NS_BEGIN

Client::Client(std::string_view name, std::string_view host, int port)
    : Client(Encoding::initName(name),
             std::make_shared<Internal::ClientData>(
                 Encoding::initName(name), Encoding::initName(host), port)) {}
Client::Client(std::wstring_view name, std::wstring_view host, int port)
    : Client(Encoding::initNameW(name),
             std::make_shared<Internal::ClientData>(
                 Encoding::initNameW(name), Encoding::initNameW(host), port)) {}

Client::Client(std::u8string_view name,
               std::shared_ptr<Internal::ClientData> data)
    : Member(data, name), data(data) {}

static std::unique_ptr<char8_t[]> initNamePtr(std::u8string_view name) {
    auto u8name = std::make_unique<char8_t[]>(name.size() + 1);
    std::copy(name.cbegin(), name.cend(), u8name.get());
    u8name[name.size()] = 0;
    return u8name;
}

Internal::ClientData::ClientData(const std::u8string &name,
                                 const std::u8string &host, int port)
    : std::enable_shared_from_this<ClientData>(), members(), fields(),
      name_mtx(), self_member_name(getMemberRef(name)), host(host), port(port),
      message_queue(std::make_shared<Common::Queue<std::string>>()),
      value_store(self_member_name), text_store(self_member_name),
      func_store(self_member_name), view_store(self_member_name),
      image_store(self_member_name), robot_model_store(self_member_name),
      canvas3d_store(self_member_name), canvas2d_store(self_member_name),
      log_store(std::make_shared<
                SyncDataStore1<std::shared_ptr<std::vector<LogLine>>>>(
          self_member_name)),
      sync_time_store(self_member_name), 
      logger_sink(std::make_shared<LoggerSink>(log_store)) {
    std::string name_str = Encoding::getName(name);
    static auto stderr_sink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks = {logger_sink, stderr_sink};
    logger =
        std::make_shared<spdlog::logger>(name_str, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger_internal = std::make_shared<spdlog::logger>(
        "webcface_internal(" + name_str + ")", stderr_sink);
    if (std::getenv("WEBCFACE_TRACE") != nullptr) {
        logger_internal->set_level(spdlog::level::trace);
    } else if (getenv("WEBCFACE_VERBOSE") != nullptr) {
        logger_internal->set_level(spdlog::level::debug);
    } else {
        logger_internal->set_level(spdlog::level::off);
    }
    logger_buf = std::make_unique<LoggerBuf>(logger);
    logger_os = std::make_unique<std::ostream>(logger_buf.get());
    log_store->setRecv(self_member_name,
                       std::make_shared<std::vector<LogLine>>());
    syncDataFirst();
}
void Internal::ClientData::start() {
    if (message_thread == nullptr) {
        message_thread = std::make_unique<std::thread>(
            Internal::messageThreadMain, shared_from_this(), host, port);
    }
    if (recv_thread == nullptr) {
        recv_thread = std::make_unique<std::thread>(Internal::recvThreadMain,
                                                    shared_from_this());
    }
}

Client::~Client() {
    close();
    data->join();
}
void Client::close() { data->closing.store(true); }
bool Client::connected() const { return data->connected.load(); }
void Internal::ClientData::join() {
    if (message_thread != nullptr && message_thread->joinable()) {
        message_thread->join();
    }
    if (recv_thread != nullptr && recv_thread->joinable()) {
        recv_thread->join();
    }
}
MemberNameRef Internal::ClientData::getMemberRef(std::u8string_view name) {
    std::lock_guard lock(name_mtx);
    for (const auto &m : members) {
        if (std::u8string_view(m.get()) == name) {
            return m.get();
        }
    }
    members.push_back(initNamePtr(name));
    return members.back().get();
}
FieldNameRef Internal::ClientData::getFieldRef(std::u8string_view name) {
    std::lock_guard lock(name_mtx);
    for (const auto &m : fields) {
        if (std::u8string_view(m.get()) == name) {
            return m.get();
        }
    }
    fields.push_back(initNamePtr(name));
    return fields.back().get();
}

std::vector<Member> Client::members() {
    std::lock_guard lock(data->entry_m);
    std::vector<Member> ret;
    for (const auto &m : data->member_entry) {
        ret.push_back(member(m));
    }
    return ret;
}
EventTarget<Member> Client::onMemberEntry() {
    std::lock_guard lock(data->event_m);
    return EventTarget<Member>{&data->member_entry_event};
}
void Client::setDefaultRunCond(FuncWrapperType wrapper) {
    data->default_func_wrapper = wrapper;
}
std::shared_ptr<LoggerSink> Client::loggerSink() { return data->logger_sink; }
std::shared_ptr<spdlog::logger> Client::logger() { return data->logger; }
LoggerBuf *Client::loggerStreamBuf() { return data->logger_buf.get(); }
std::ostream &Client::loggerOStream() { return *data->logger_os.get(); }
std::string Client::serverVersion() const { return data->svr_version; }
std::string Client::serverName() const { return data->svr_name; }
FuncListener Client::funcListener(std::u8string_view field) const {
    return FuncListener{*this, field};
}

void Internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        message_queue->push(Message::packSingle(Message::PingStatusReq{}));
    }
    ping_status_req = true;
}

void Internal::recvThreadMain(std::shared_ptr<ClientData> data) {
    while (!data->closing.load()) {
        auto msg = data->recv_queue.pop(std::chrono::milliseconds(10));
        if (msg) {
            data->onRecv(*msg);
        }
    }
}

void Client::start() {
    if (!connected()) {
        data->start();
    }
}
void Client::waitConnection() {
    if (!connected()) {
        data->start();
        std::unique_lock lock(data->connect_state_m);
        data->connect_state_cond.wait(lock, [this] { return connected(); });
    }
}
void Internal::ClientData::syncDataFirst() {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard image_lock(image_store.mtx);
    std::lock_guard robot_model_lock(robot_model_store.mtx);
    std::lock_guard canvas3d_lock(canvas3d_store.mtx);
    std::lock_guard canvas2d_lock(canvas2d_store.mtx);
    std::lock_guard log_lock(log_store->mtx);

    std::stringstream buffer;
    int len = 0;

    Message::pack(
        buffer, len,
        Message::SyncInit{{},
                          std::u8string(Encoding::getNameU8(self_member_name)),
                          0,
                          "cpp",
                          WEBCFACE_VERSION,
                          ""});

    for (const auto &v : value_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::Value>{
                              {},
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second});
        }
    }
    for (const auto &v : text_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::Text>{
                              {},
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second});
        }
    }
    for (const auto &v : view_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::View>{
                              {},
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second});
        }
    }
    for (const auto &v : robot_model_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::RobotModel>{
                              {},
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second});
        }
    }
    for (const auto &v : canvas3d_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::Canvas3D>{
                              {},
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second});
        }
    }
    for (const auto &v : canvas2d_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::Canvas2D>{
                              {},
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second});
        }
    }
    for (const auto &v : image_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(buffer, len,
                          Message::Req<Message::Image>{
                              std::u8string(Encoding::getNameU8(v.first)),
                              std::u8string(Encoding::getNameU8(v2.first)),
                              v2.second,
                              image_store.getReqInfo(v.first, v2.first)});
        }
    }
    for (const auto &v : log_store->transferReq()) {
        Message::pack(
            buffer, len,
            Message::LogReq{{}, std::u8string(Encoding::getNameU8(v.first))});
    }

    syncData(true);

    if (ping_status_req) {
        Message::pack(buffer, len, Message::PingStatusReq{});
    }

    message_queue->push(Message::packDone(buffer, len));
}
void Internal::ClientData::syncData(bool is_first) {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard image_lock(image_store.mtx);
    std::lock_guard robot_model_lock(robot_model_store.mtx);
    std::lock_guard canvas3d_lock(canvas3d_store.mtx);
    std::lock_guard canvas2d_lock(canvas2d_store.mtx);
    std::lock_guard log_lock(log_store->mtx);

    std::stringstream buffer;
    int len = 0;

    Message::pack(buffer, len, Message::Sync{});

    for (const auto &v : value_store.transferSend(is_first)) {
        Message::pack(
            buffer, len,
            Message::Value{
                {},
                std::u8string(Encoding::getNameU8(v.first)),
                std::static_pointer_cast<std::vector<double>>(v.second)});
    }
    for (const auto &v : text_store.transferSend(is_first)) {
        Message::pack(buffer, len,
                      Message::Text{{},
                                    std::u8string(Encoding::getNameU8(v.first)),
                                    v.second});
    }
    for (const auto &v : robot_model_store.transferSend(is_first)) {
        Message::pack(
            buffer, len,
            Message::RobotModel{std::u8string(Encoding::getNameU8(v.first)),
                                v.second});
    }
    auto view_prev = view_store.getSendPrev(is_first);
    for (const auto &p : view_store.transferSend(is_first)) {
        auto v_prev = view_prev.find(p.first);
        auto v_diff = view_store.getDiff(
            p.second.get(),
            v_prev == view_prev.end() ? nullptr : v_prev->second.get());
        if (!v_diff->empty()) {
            Message::pack(
                buffer, len,
                Message::View{std::u8string(Encoding::getNameU8(p.first)),
                              v_diff, p.second->size()});
        }
    }
    auto canvas3d_prev = canvas3d_store.getSendPrev(is_first);
    for (const auto &p : canvas3d_store.transferSend(is_first)) {
        auto v_prev = canvas3d_prev.find(p.first);
        auto v_diff = canvas3d_store.getDiff(
            p.second.get(),
            v_prev == canvas3d_prev.end() ? nullptr : v_prev->second.get());
        if (!v_diff->empty()) {
            Message::pack(
                buffer, len,
                Message::Canvas3D{std::u8string(Encoding::getNameU8(p.first)),
                                  v_diff, p.second->size()});
        }
    }
    auto canvas2d_prev = canvas2d_store.getSendPrev(is_first);
    for (const auto &p : canvas2d_store.transferSend(is_first)) {
        auto v_prev = canvas2d_prev.find(p.first);
        auto v_diff = view_store.getDiff(&p.second->components,
                                         v_prev == canvas2d_prev.end()
                                             ? nullptr
                                             : &v_prev->second->components);
        if (!v_diff->empty()) {
            Message::pack(
                buffer, len,
                Message::Canvas2D{std::u8string(Encoding::getNameU8(p.first)),
                                  p.second->width, p.second->height, v_diff,
                                  p.second->components.size()});
        }
    }
    for (const auto &v : image_store.transferSend(is_first)) {
        Message::pack(
            buffer, len,
            Message::Image{std::u8string(Encoding::getNameU8(v.first)),
                           v.second});
    }

    auto log_s = *log_store->getRecv(self_member_name);
    if ((log_s->size() > 0 && is_first) || log_s->size() > log_sent_lines) {
        auto begin = log_s->begin();
        auto end = log_s->end();
        if (!is_first) {
            begin += log_sent_lines;
        }
        log_sent_lines = log_s->size();
        Message::pack(buffer, len, Message::Log{begin, end});
    }
    for (const auto &v : func_store.transferSend(is_first)) {
        if (!Encoding::getNameU8(v.first).starts_with(u8'.')) {
            Message::pack(
                buffer, len,
                Message::FuncInfo{std::u8string(Encoding::getNameU8(v.first)),
                                  *v.second});
        }
    }

    message_queue->push(Message::packDone(buffer, len));
}
void Client::sync() {
    start();
    data->syncData(false);
    while (auto func_sync = data->func_sync_queue.pop()) {
        (*func_sync)->sync();
    }
}
void Internal::ClientData::onRecv(const std::string &message) {
    static std::unordered_map<int, bool> message_kind_warned;
    namespace MessageKind = webcface::Message::MessageKind;
    auto messages = webcface::Message::unpack(message, this->logger_internal);
    std::vector<MemberNameRef> sync_members;
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::svr_version: {
            auto r = std::any_cast<webcface::Message::SvrVersion>(obj);
            this->svr_name = r.svr_name;
            this->svr_version = r.ver;
            break;
        }
        case MessageKind::ping: {
            this->message_queue->push(
                webcface::Message::packSingle(webcface::Message::Ping{}));
            break;
        }
        case MessageKind::ping_status: {
            auto r = std::any_cast<webcface::Message::PingStatus>(obj);
            this->ping_status = r.status;
            std::unordered_set<std::string> members;
            {
                std::lock_guard lock(entry_m);
                members = this->member_entry;
            }
            for (const auto &member_name : members) {
                eventpp::CallbackList<void(Member)> *cl = nullptr;
                {
                    std::lock_guard lock(event_m);
                    if (this->ping_event.count(member_name)) {
                        cl = &this->ping_event.at(member_name);
                    }
                }
                if (cl) {
                    cl->operator()(Field{shared_from_this(), member_name});
                }
            }
            break;
        }
        case MessageKind::sync: {
            auto r = std::any_cast<webcface::Message::Sync>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->sync_time_store.setRecv(member, r.getTime());
            sync_members.push_back(member);
            break;
        }
        case MessageKind::value + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::Value>>(
                    obj);
            auto [member, field_u8] =
                this->value_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            this->value_store.setRecv(
                member, field,
                std::static_pointer_cast<VectorOpt<double>>(r.data));
            eventpp::CallbackList<void(Value)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->value_change_event.count(key)) {
                    cl = &this->value_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::Text>>(
                    obj);
            auto [member, field_u8] =
                this->text_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            this->text_store.setRecv(member, field, r.data);
            eventpp::CallbackList<void(Text)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->text_change_event.count(key)) {
                    cl = &this->text_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::robot_model + MessageKind::res: {
            auto r = std::any_cast<
                webcface::Message::Res<webcface::Message::RobotModel>>(obj);
            auto [member, field_u8] =
                this->robot_model_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            this->robot_model_store.setRecv(member, field, r.commonLinks());
            eventpp::CallbackList<void(RobotModel)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->robot_model_change_event.count(key)) {
                    cl = &this->robot_model_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::View>>(
                    obj);
            std::lock_guard lock(this->view_store.mtx);
            auto [member, field_u8] =
                this->view_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            auto v_prev = this->view_store.getRecv(member, field);
            if (v_prev == std::nullopt) {
                v_prev =
                    std::make_shared<std::vector<ViewComponentBase>>(r.length);
                this->view_store.setRecv(member, field, *v_prev);
            }
            (*v_prev)->resize(r.length);
            for (const auto &d : *r.data_diff) {
                (**v_prev)[std::stoi(d.first)] = d.second;
            }
            eventpp::CallbackList<void(View)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->view_change_event.count(key)) {
                    cl = &this->view_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::canvas3d + MessageKind::res: {
            auto r = std::any_cast<
                webcface::Message::Res<webcface::Message::Canvas3D>>(obj);
            std::lock_guard lock(this->canvas3d_store.mtx);
            auto [member, field_u8] =
                this->canvas3d_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            auto v_prev = this->canvas3d_store.getRecv(member, field);
            if (v_prev == std::nullopt) {
                v_prev = std::make_shared<std::vector<Canvas3DComponentBase>>(
                    r.length);
                this->canvas3d_store.setRecv(member, field, *v_prev);
            }
            (*v_prev)->resize(r.length);
            for (const auto &d : *r.data_diff) {
                (**v_prev)[std::stoi(d.first)] = d.second;
            }
            eventpp::CallbackList<void(Canvas3D)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->canvas3d_change_event.count(key)) {
                    cl = &this->canvas3d_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::canvas2d + MessageKind::res: {
            auto r = std::any_cast<
                webcface::Message::Res<webcface::Message::Canvas2D>>(obj);
            std::lock_guard lock(this->canvas2d_store.mtx);
            auto [member, field_u8] =
                this->canvas2d_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            auto v_prev = this->canvas2d_store.getRecv(member, field);
            if (v_prev == std::nullopt) {
                v_prev = std::make_shared<Canvas2DDataBase>();
                this->canvas2d_store.setRecv(member, field, *v_prev);
            }
            (*v_prev)->width = r.width;
            (*v_prev)->height = r.height;
            (*v_prev)->components.resize(r.length);
            for (const auto &d : *r.data_diff) {
                (*v_prev)->components[std::stoi(d.first)] = d.second;
            }
            eventpp::CallbackList<void(Canvas2D)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->canvas2d_change_event.count(key)) {
                    cl = &this->canvas2d_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::image + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::Image>>(
                    obj);
            auto [member, field_u8] =
                this->image_store.getReq(r.req_id, r.sub_field);
            auto field = getFieldRef(field_u8);
            this->image_store.setRecv(member, field, r);
            eventpp::CallbackList<void(Image)> *cl = nullptr;
            FieldBaseComparable key{member, field};
            {
                std::lock_guard lock(event_m);
                if (this->image_change_event.count(key)) {
                    cl = &this->image_change_event.at(key);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::log: {
            auto r = std::any_cast<webcface::Message::Log>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            std::lock_guard lock(this->log_store->mtx);
            auto log_s = this->log_store->getRecv(member);
            if (!log_s) {
                log_s = std::make_shared<std::vector<LogLine>>();
                this->log_store->setRecv(member, *log_s);
            }
            for (const auto &lm : *r.log) {
                (*log_s)->push_back(lm);
            }
            eventpp::CallbackList<void(Log)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->log_append_event.count(member)) {
                    cl = &this->log_append_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member});
            }
            break;
        }
        case MessageKind::call: {
            auto r = std::any_cast<webcface::Message::Call>(obj);
            std::thread([data = shared_from_this(), r] {
                auto func_info = data->func_store.getRecv(
                    data->self_member_name, data->getFieldRef(r.field));
                if (func_info) {
                    data->message_queue->push(webcface::Message::packSingle(
                        webcface::Message::CallResponse{
                            {}, r.caller_id, r.caller_member_id, true}));
                    ValAdaptor result;
                    bool is_error = false;
                    try {
                        result = (*func_info)->run(r.args);
                    } catch (const std::exception &e) {
                        is_error = true;
                        result = std::string(e.what());
                    } catch (const std::string &e) {
                        is_error = true;
                        result = e;
                    } catch (...) {
                        is_error = true;
                        result = "unknown exception";
                    }
                    data->message_queue->push(webcface::Message::packSingle(
                        webcface::Message::CallResult{{},
                                                      r.caller_id,
                                                      r.caller_member_id,
                                                      is_error,
                                                      result}));
                } else {
                    data->message_queue->push(webcface::Message::packSingle(
                        webcface::Message::CallResponse{
                            {}, r.caller_id, r.caller_member_id, false}));
                }
            }).detach();
            break;
        }
        case MessageKind::call_response: {
            auto r = std::any_cast<webcface::Message::CallResponse>(obj);
            try {
                this->func_result_store.resultSetter(r.caller_id)
                    .setStarted(r.started);
                if (!r.started) {
                    this->func_result_store.removeResultSetter(r.caller_id);
                }
            } catch (const std::future_error &e) {
                this->logger_internal->error(
                    "error receiving call response id={}: {}", r.caller_id,
                    e.what());
            } catch (const std::out_of_range &e) {
                this->logger_internal->error(
                    "error receiving call response id={}: {}", r.caller_id,
                    e.what());
            }
            break;
        }
        case MessageKind::call_result: {
            auto r = std::any_cast<webcface::Message::CallResult>(obj);
            try {
                if (r.is_error) {
                    try {
                        throw std::runtime_error(
                            static_cast<std::string>(r.result));
                    } catch (...) {
                        this->func_result_store.resultSetter(r.caller_id)
                            .setResultException(std::current_exception());
                    }
                } else {
                    this->func_result_store.resultSetter(r.caller_id)
                        .setResult(r.result);
                    // todo: 戻り値の型?
                }
                this->func_result_store.removeResultSetter(r.caller_id);
            } catch (const std::future_error &e) {
                this->logger_internal->error(
                    "error receiving call result id={}: {}", r.caller_id,
                    e.what());
            } catch (const std::out_of_range &e) {
                this->logger_internal->error(
                    "error receiving call response id={}: {}", r.caller_id,
                    e.what());
            }
            break;
        }
        case MessageKind::sync_init: {
            auto r = std::any_cast<webcface::Message::SyncInit>(obj);
            {
                std::lock_guard lock(this->entry_m);
                this->member_entry.emplace(r.member_name);
            }
            this->value_store.clearEntry(r.member_name);
            this->text_store.clearEntry(r.member_name);
            this->func_store.clearEntry(r.member_name);
            this->view_store.clearEntry(r.member_name);
            this->image_store.clearEntry(r.member_name);
            this->robot_model_store.clearEntry(r.member_name);
            this->canvas3d_store.clearEntry(r.member_name);
            this->canvas2d_store.clearEntry(r.member_name);
            this->member_ids[r.member_name] = r.member_id;
            this->member_lib_name[r.member_id] = r.lib_name;
            this->member_lib_ver[r.member_id] = r.lib_ver;
            this->member_addr[r.member_id] = r.addr;
            this->member_entry_event(Field{shared_from_this(), r.member_name});
            break;
        }
        case MessageKind::entry + MessageKind::value: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Value>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->value_store.setEntry(member, r.field);
            eventpp::CallbackList<void(Value)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->value_entry_event.count(member)) {
                    cl = &this->value_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Text>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->text_store.setEntry(member, r.field);
            eventpp::CallbackList<void(Text)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->text_entry_event.count(member)) {
                    cl = &this->text_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::View>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->view_store.setEntry(member, r.field);
            eventpp::CallbackList<void(View)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->view_entry_event.count(member)) {
                    cl = &this->view_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::entry + MessageKind::canvas3d: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Canvas3D>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->canvas3d_store.setEntry(member, r.field);
            eventpp::CallbackList<void(Canvas3D)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->canvas3d_entry_event.count(member)) {
                    cl = &this->canvas3d_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::entry + MessageKind::canvas2d: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Canvas2D>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->canvas2d_store.setEntry(member, r.field);
            eventpp::CallbackList<void(Canvas2D)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->canvas2d_entry_event.count(member)) {
                    cl = &this->canvas2d_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::entry + MessageKind::robot_model: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::RobotModel>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->robot_model_store.setEntry(member, r.field);
            eventpp::CallbackList<void(RobotModel)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->robot_model_entry_event.count(member)) {
                    cl = &this->robot_model_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::entry + MessageKind::image: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Image>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->image_store.setEntry(member, r.field);
            eventpp::CallbackList<void(Image)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->image_entry_event.count(member)) {
                    cl = &this->image_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::func_info: {
            auto r = std::any_cast<webcface::Message::FuncInfo>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            auto field = getFieldRef(r.field);
            this->func_store.setEntry(member, field);
            this->func_store.setRecv(member, field,
                                     std::make_shared<FuncInfo>(r));
            eventpp::CallbackList<void(Func)> *cl = nullptr;
            {
                std::lock_guard lock(event_m);
                if (this->func_entry_event.count(member)) {
                    cl = &this->func_entry_event.at(member);
                }
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::value:
        case MessageKind::text:
        case MessageKind::view:
        case MessageKind::canvas3d:
        case MessageKind::canvas2d:
        case MessageKind::robot_model:
        case MessageKind::image:
        case MessageKind::value + MessageKind::req:
        case MessageKind::text + MessageKind::req:
        case MessageKind::view + MessageKind::req:
        case MessageKind::canvas3d + MessageKind::req:
        case MessageKind::canvas2d + MessageKind::req:
        case MessageKind::robot_model + MessageKind::req:
        case MessageKind::image + MessageKind::req:
        case MessageKind::ping_status_req:
        case MessageKind::log_req:
            if (!message_kind_warned[kind]) {
                logger->warn("Invalid Message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        case MessageKind::unknown:
            break;
        default:
            if (!message_kind_warned[kind]) {
                logger->warn("Unknown Message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        }
    }
    for (const auto &m : sync_members) {
        eventpp::CallbackList<void(Member)> *cl = nullptr;
        {
            std::lock_guard lock(event_m);
            if (this->sync_event.count(m)) {
                cl = &this->sync_event.at(m);
            }
        }
        if (cl) {
            cl->operator()(Field{shared_from_this(), m});
        }
    }
}
WEBCFACE_NS_END
