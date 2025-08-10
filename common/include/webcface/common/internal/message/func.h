#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include "webcface/common/val_adaptor_vec.h"
#include <optional>
#include <vector>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

/*!
 * \brief 関数呼び出し (client(caller)->server->client(receiver))
 *
 * caller側clientが一意のcaller_idを振る(0以上の整数)
 *
 * serverはcaller_member_idをつけてreceiverに送る
 *
 */
struct Call : public MessageBase<MessageKind::call> {
    using CallerId = std::size_t;
    using MemberId = unsigned int;

    CallerId caller_id = 0;
    MemberId caller_member_id = 0;
    MemberId target_member_id = 0;
    SharedString field;
    std::vector<webcface::ValAdaptorVector> args;
    Call() = default;
    Call(CallerId caller_id, MemberId caller_member_id,
         MemberId target_member_id, const SharedString &field,
         const std::vector<ValAdaptorVector> &args)
        : message::MessageBase<MessageKind::call>(), caller_id(caller_id),
          caller_member_id(caller_member_id),
          target_member_id(target_member_id), field(field), args(args) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("r", target_member_id),
                       MSGPACK_NVP("f", field), MSGPACK_NVP("a", args))
};
/*!
 * \brief 関数呼び出しの応答1 (client(receiver)->server->client(caller))
 *
 * clientはcalled_id,caller_member_idと、関数の実行を開始したかどうかを返す
 *
 * * 関数の実行に時間がかかる場合も実行完了を待たずにstartedをtrueにして送る
 * * 対象の関数が存在しない場合、startedをfalseにして送る
 *
 * serverはそれをそのままcallerに送る
 *
 */
struct CallResponse : public MessageBase<MessageKind::call_response> {
    std::size_t caller_id;
    unsigned int caller_member_id;
    bool started; //!< 関数の実行を開始したかどうか
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("s", started))
};
/*!
 * \brief 関数呼び出しの応答2 (client(receiver)->server->client(caller))
 *
 * clientはcalled_id,caller_member_idと、関数の実行結果を返す
 *
 * resultに結果を文字列または数値または真偽値で返す
 * * 結果が無い場合は "" を返す
 * * 例外が発生した場合はis_errorをtrueにしresultに例外の内容を文字列で入れて返す
 *
 * serverはそれをそのままcallerに送る
 *
 */
struct CallResult : public MessageBase<MessageKind::call_result> {
    std::size_t caller_id;
    unsigned int caller_member_id;
    bool is_error;
    ValAdaptor result;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", caller_id),
                       MSGPACK_NVP("c", caller_member_id),
                       MSGPACK_NVP("e", is_error), MSGPACK_NVP("r", result))
};

/*!
 * \brief client(member)->server->client func登録
 *
 * client->server時はmemberは無視
 *
 */
struct Arg {
    SharedString name_;
    ValType type_ = ValType::none_;
    std::optional<ValAdaptor> init_ = std::nullopt;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt;
    std::vector<ValAdaptor> option_;
    Arg() = default;
    Arg(const SharedString &name, ValType type,
        const std::optional<ValAdaptor> &init,
        const std::optional<double> &min_, const std::optional<double> &max_,
        const std::vector<ValAdaptor> &option)
        : name_(name), type_(type), init_(init), min_(min_), max_(max_),
          option_(option) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name_), MSGPACK_NVP("t", type_),
                       MSGPACK_NVP("i", init_), MSGPACK_NVP("m", min_),
                       MSGPACK_NVP("x", max_), MSGPACK_NVP("o", option_))
};
struct FuncInfo : public MessageBase<MessageKind::func_info> {
    unsigned int member_id = 0;
    SharedString field;
    ValType return_type;
    std::vector<std::shared_ptr<Arg>> args;
    int index = 0;
    FuncInfo() = default;
    FuncInfo(unsigned int member_id, const SharedString &field,
             ValType return_type, const std::vector<std::shared_ptr<Arg>> &args,
             int index)
        : member_id(member_id), field(field), return_type(return_type),
          args(args), index(index) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("r", return_type), MSGPACK_NVP("a", args),
                       MSGPACK_NVP("i", index))
};

} // namespace message
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::FuncInfo)
WEBCFACE_MESSAGE_FMT(webcface::message::Call)
WEBCFACE_MESSAGE_FMT(webcface::message::CallResponse)
WEBCFACE_MESSAGE_FMT(webcface::message::CallResult)
