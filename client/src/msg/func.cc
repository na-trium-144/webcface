#include "webcface/internal/client_internal.h"
#include "webcface/message/func.h"
#include "webcface/func.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_func(int kind,
                                       const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::call: {
        auto &r = *static_cast<webcface::message::Call *>(obj.get());
        auto func_info =
            this->func_store.getRecv(this->self_member_name, r.field);
        if (func_info) {
            this->messagePushAlways(
                webcface::message::packSingle(webcface::message::CallResponse{
                    {}, r.caller_id, r.caller_member_id, true}));
            (*func_info)->run(std::move(r));
        } else {
            this->messagePushAlways(
                webcface::message::packSingle(webcface::message::CallResponse{
                    {}, r.caller_id, r.caller_member_id, false}));
        }
        break;
    }
    case MessageKind::call_response: {
        auto &r = *static_cast<webcface::message::CallResponse *>(obj.get());
        try {
            this->func_result_store.getResult(r.caller_id)
                ->setter()
                .reach(r.started);
            if (!r.started) {
                this->func_result_store.removeResult(r.caller_id);
            }
        } catch (const std::runtime_error &e) {
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
        auto &r = *static_cast<webcface::message::CallResult *>(obj.get());
        try {
            if (r.is_error) {
                this->func_result_store.getResult(r.caller_id)
                    ->setter()
                    .reject(r.result);
            } else {
                this->func_result_store.getResult(r.caller_id)
                    ->setter()
                    .respond(r.result);
                // todo: 戻り値の型?
            }
            this->func_result_store.removeResult(r.caller_id);
        } catch (const std::runtime_error &e) {
            this->logger_internal->error(
                "error receiving call result id={}: {}", r.caller_id, e.what());
        } catch (const std::out_of_range &e) {
            this->logger_internal->error(
                "error receiving call response id={}: {}", r.caller_id,
                e.what());
        }
        break;
    }
    case MessageKind::func_info: {
        auto &r = *static_cast<webcface::message::FuncInfo *>(obj.get());
        auto member = this->getMemberNameFromId(r.member_id);
        this->func_store.setEntry(member, r.field);
        this->func_store.setRecv(member, r.field,
                                 std::make_shared<FuncInfo>(r));
        std::shared_ptr<std::function<void(Func)>> cl;
        {
            std::lock_guard lock(event_m);
            cl = findFromMap1(this->func_entry_event, member).value_or(nullptr);
        }
        if (cl && *cl) {
            cl->operator()(Field{shared_from_this(), member, r.field});
        }
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}

void internal::ClientData::packSyncData_func(std::stringstream &buffer,
                                             int &len,
                                             const SyncDataSnapshot &data) {
    for (const auto &v : data.func_data) {
        if (!v.first.startsWith(field_separator)) {
            message::pack(buffer, len, v.second->toMessage(v.first));
        }
    }
}

WEBCFACE_NS_END
