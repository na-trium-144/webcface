#pragma once
#include "webcface/message/base.h"

WEBCFACE_NS_BEGIN
namespace message {
struct LogLine {
    int level_ = 0;
    /*!
     * \brief 1970/1/1からの経過ミリ秒
     *
     * コンストラクタで初期化、data()でtime_pointに戻す
     *
     */
    std::uint64_t time_ms = 0;
    SharedString message_;
    LogLine() = default;
    LogLine(int level, std::uint64_t time_ms, const SharedString &message)
        : level_(level), time_ms(time_ms), message_(message) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("v", level_), MSGPACK_NVP("t", time_ms),
                       MSGPACK_NVP("m", message_))
};
/*!
 * \brief client(member)->server->client logを追加
 *
 * client->server時はmemberは無視
 *
 */
struct LogDefault : public MessageBase<MessageKind::log_default> {
    unsigned int member_id = 0;
    std::shared_ptr<std::deque<LogLine>> log;
    LogDefault() = default;
    LogDefault(unsigned int member_id,
               const std::shared_ptr<std::deque<LogLine>> &log)
        : member_id(member_id), log(log) {}
    template <typename It>
    LogDefault(const It &begin, const It &end) : member_id(0) {
        this->log = std::make_shared<std::deque<LogLine>>();
        for (auto it = begin; it < end; it++) {
            if constexpr (std::is_same_v<decltype(*it), LogLine>) {
                this->log->push_back(*it);
            } else {
                this->log->emplace_back(it->toMessage());
            }
        }
    }
    explicit LogDefault(const LogLine &ll) : member_id(0) {
        this->log = std::make_shared<std::deque<LogLine>>(1);
        this->log->front() = ll;
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("l", log))
};
/*!
 * \brief client(member)->server->client logを追加
 * \since ver2.4
 *
 */
struct Log : public MessageBase<MessageKind::log> {
    static SharedString defaultLogName();

    SharedString field;
    std::shared_ptr<std::deque<LogLine>> log;
    Log() = default;
    Log(const SharedString &field,
        const std::shared_ptr<std::deque<LogLine>> &log)
        : field(field), log(log) {}
    template <typename It>
    Log(const SharedString &field, const It &begin, const It &end)
        : field(field) {
        this->log = std::make_shared<std::deque<LogLine>>();
        for (auto it = begin; it < end; it++) {
            if constexpr (std::is_same_v<decltype(*it), LogLine>) {
                this->log->push_back(*it);
            } else {
                this->log->emplace_back(it->toMessage());
            }
        }
    }
    explicit Log(const SharedString &field, const LogLine &ll) : field(field) {
        this->log = std::make_shared<std::deque<LogLine>>(1);
        this->log->front() = ll;
    }
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("l", log))
};
struct LogReqDefault : public MessageBase<MessageKind::log_req_default> {
    SharedString member;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("M", member))
};
struct LogEntryDefault : public MessageBase<MessageKind::log_entry_default> {
    unsigned int member_id = 0;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id))
};

template <>
struct Res<Log> : public MessageBase<MessageKind::log + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::shared_ptr<std::deque<LogLine>> log;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::shared_ptr<std::deque<LogLine>> &log)
        : MessageBase<MessageKind::log + MessageKind::res>(), req_id(req_id),
          sub_field(sub_field), log(log) {}
    template <typename It>
    Res(unsigned int req_id, const SharedString &sub_field, const It &begin,
        const It &end)
        : MessageBase<MessageKind::log + MessageKind::res>(), req_id(req_id),
          sub_field(sub_field) {
        this->log = std::make_shared<std::deque<LogLine>>();
        for (auto it = begin; it < end; it++) {
            if constexpr (std::is_same_v<decltype(*it), LogLine>) {
                this->log->push_back(*it);
            } else {
                this->log->emplace_back(it->toMessage());
            }
        }
    }

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("l", log))
};


} // namespace message
WEBCFACE_NS_END
