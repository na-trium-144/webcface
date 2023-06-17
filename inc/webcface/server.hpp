#pragma once
#include <cstddef>  //size_t
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <jsoncpp/json/json.h>
#include <webcface/type.hpp>

namespace WebCFace
{
inline namespace Server
{
extern std::mutex callback_mutex;
extern std::mutex internal_mutex;
void startServer(int port = 80, const std::string& static_dir = WEBCFACE_STATIC_DIR);
void quitServer();
void updateSetting();
void sendData();

void setServerName(const std::string& name);
void addRelatedServer(int port);
void addRelatedServer(const std::string& addr, int port);

// ブラウザに最大何Hzで情報を送るか
// （これ以上の頻度でsendData()を読んでも、実際にはsendされない場合がある (default: 30Hz)
void setMaxSendDataFrequency(int);
int getMaxSendDataFrequency();

template <typename CallbackT>
struct RobotInfoBase {
    std::function<CallbackT> callback;
    std::vector<std::string> names;
    std::vector<ValueType> types;
    std::vector<Json::Value> default_values;
};
struct ImageInfo {
    Json::Value src;
    bool has_changed = true;
};
using ToRobotInfo = RobotInfoBase<void(const std::string&)>;
using FromRobotInfo = RobotInfoBase<Json::Value()>;

extern std::unordered_map<std::string, ToRobotInfo> to_robot_var, to_robot_func;
extern std::unordered_map<std::string, FromRobotInfo> from_robot;
extern std::unordered_map<std::string, ImageInfo> images;
extern std::unordered_map<std::string, Json::Value> custom_page_layout;
extern std::unordered_map<std::string, Json::Value> drawing_layer;
extern std::vector<std::string> button_name, axis_name;
extern bool setting_changed;

void dialog(const std::string& alert_name);

std::int64_t getTime();
}  // namespace Server
}  // namespace WebCFace
