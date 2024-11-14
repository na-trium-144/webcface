#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace message {
// 新しいメッセージの定義は
// kind追記→struct作成→message.ccに追記→s_client_data.ccに追記→client.ccに追記

namespace MessageKind {
enum MessageKindEnum {
    unknown = -1,
    value = 0,
    text = 1,
    binary = 2,
    view_old = 3,
    canvas2d_old = 4,
    image = 5,
    robot_model = 6,
    canvas3d_old = 7,
    log = 8,
    view = 9,
    canvas2d = 10,
    canvas3d = 11,
    entry = 20,
    req = 40,
    res = 60,
    sync_init = 80,
    call = 81,
    call_response = 82,
    call_result = 83,
    func_info = 84,
    log_default = 85,
    log_req_default = 86,
    sync = 87,
    sync_init_end = 88,
    // svr_version = 88,
    ping = 89,
    ping_status = 90,
    ping_status_req = 91,
    log_entry_default = 92,
};
}

/*!
 * \brief 型からkindを取得するためだけのベースクラス
 *
 */
template <int k>
struct MessageBase {
    static constexpr int kind = k;
};

template <typename T>
struct Res {};

} // namespace message
WEBCFACE_NS_END
