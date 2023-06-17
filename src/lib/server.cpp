#include "server.hpp"
#include "host.hpp"
#include "logger.hpp"
#include <drogon/drogon.h>
#include <iostream>
#include <json/json.h>
#include <thread>
#include <filesystem>
#include <fstream>
#include <exception>
#include <optional>
#include <chrono>
// #include <webcface/registration.hpp>

namespace WebCFace
{
inline namespace Server
{
// ライブラリ外から読むのでinlineだとだめっぽい?
// いやそんなことなさそう?
// よくわからんけどexternにしてcpp内で定義したら動いたのでよし

//! Callbackを実行時にlockするmutex
/*! functionToRobotで登録された関数を実行するときにこのmutexがlockされます。\n
 * 関数を呼び出してほしくない間これをlockしておくことで関数の呼び出しを防げます\n
 * functionFromRobotで登録した関数はこのmutexの状態に関係なく SendData() 時に実行されます\n
 */
std::mutex callback_mutex;

//! jsonオブジェクトやその他各種設定を同時に触らないようにする
//! todo: このmutexを外から操作されたらどーすんねん
std::mutex internal_mutex;
std::unordered_map<std::string, ToRobotInfo> to_robot_var, to_robot_func;
std::unordered_map<std::string, FromRobotInfo> from_robot;
std::unordered_map<std::string, ImageInfo> images;
std::unordered_map<std::string, Json::Value> custom_page_layout;
std::vector<std::string> button_name, axis_name;

//! drogon起動中はdrogonが動いているスレッド、停止中はstd::nullopt
std::optional<std::thread> drogon_main_thread;

//! サーバーを起動する
/*! ポート80でサーバーを起動します\n
 * 自分のpcで動かす場合、ポート80だとsudoが必要になるので8000とかに変えると良いです\n
 * \param port サーバーを起動するポート
 * \param static_dir フロントエンドのhtmlファイルのディレクトリ\n
 * 省略時は自動でWebCFaceのフロントエンドが使用されます\n
 * 絶対パスを書いた場合は絶対パス、相対パスを書いた場合はwebcfaceディレクトリからの相対パスになります\n
 */
void startServer(int port, const std::string& static_dir)
{
    if (drogon_main_thread) {
        std::cerr << "[WebCFace] startServer() called but server has already been started"
                  << std::endl;
        return;
    }
    {
        std::lock_guard lock(internal_mutex);
        server_port = port;
        std::cout << "[WebCFace] version " << WEBCFACE_VERSION << ", server start" << std::endl;
        std::cout << "           url = http://" << host.addr << ":" << port << "/" << std::endl;
        std::cout << "           staticdir = " << static_dir << std::endl;
        std::ofstream drogon_config_file(WEBCFACE_CONFIG_PATH);
        drogon_config_file << "{\"app\":{\"document_root\":\"" << static_dir << "\"}}" << std::endl;
        drogon_config_file.close();
        // Set HTTP listener address and port
        drogon::app().addListener("0.0.0.0", port);
        // Load config file
        drogon::app().loadConfigFile(WEBCFACE_CONFIG_PATH);
        drogon::app().disableSigtermHandling();
        drogon::app().registerPostHandlingAdvice(
            [](const drogon::HttpRequestPtr& req, const drogon::HttpResponsePtr& resp) {
                // LOG_DEBUG << "postHandling1";
                resp->addHeader("Access-Control-Allow-Origin", "*");
            });

        // 再接続画面以外で接続すると404になるので、404ページをカスタマイズして自動リダイレクトするようにする
        // issues#57
        // https://github.com/drogonframework/drogon/issues/424
        auto error404 = drogon::HttpResponse::newHttpResponse();
        error404->setStatusCode(drogon::k404NotFound);
        error404->setContentTypeCode(drogon::CT_TEXT_HTML);
        error404->setBody(R"(
    <body>
      <h1>404 Drogon Not Found</h1>
      <p>redirect to = <span id="url"></span></p>
      <script>
        const url = window.location.pathname;
        console.log(url);
        document.getElementById("url").textContext = url;
        window.location.href = "/?redirect="+url;
      </script>
    </body>
    )");
        error404->setExpiredTime(0);
        drogon::app().setCustom404Page(error404);

        drogon_main_thread = std::make_optional<std::thread>([]() {
            // Run HTTP framework,the method will block in the internal event loop
            drogon::app().run();
        });
    }
    //! quitServerを自動で呼ぶstruct
    static struct QuitServerOnDestruct {
        ~QuitServerOnDestruct() { quitServer(); }
    } q;
}


//! サーバーを停止する
/*! サーバーを停止します\n
 * プログラムが正常終了(exitとか?)する場合はデストラクタで勝手に呼ばれるので、必ずしも明示的にquitServerする必要はない\n
 * (quitServerした後にstartServerするとなぜかセグフォすることが知られているので、これを呼び出す意味はほぼ無いです)\n
 */
void quitServer()
{
    if (drogon_main_thread) {
        std::lock_guard lock(internal_mutex);
        std::cout << "[WebCFace] quit server" << std::endl;
        clients.clear();
        // drogon::app().quit();
        drogon::app().getLoop()->queueInLoop([]() { drogon::app().quit(); });
        drogon_main_thread->join();
        drogon_main_thread = std::nullopt;
    }
}

//! サーバー名を設定する
/*! \param name サーバー名\n
 * 半角コロンは使わないでください
 */
void setServerName(const std::string& name)
{
    std::lock_guard lock(internal_mutex);
    server_name = name;
    setting_changed = true;
}

//! フロントエンドが同時接続するサーバーの情報を追加(アドレス省略)
/*! アドレスがこのサーバーと同じ場合アドレスを省略できます
 * \param port ポート番号
 */
void addRelatedServer(int port)
{
    addRelatedServer(host.addr, port);
}
//! フロントエンドが同時接続するサーバーの情報を追加
/*! アドレスがこのサーバーと同じ場合アドレスを省略できます
 * \param addr アドレス
 * \param port ポート番号
 */
void addRelatedServer(const std::string& addr, int port)
{
    std::lock_guard lock(internal_mutex);
    related_servers.push_back({addr, port});
    setting_changed = true;
}

//! 登録された関数名の一覧を取得(これ要る?)
std::vector<std::string> getAllRegisteredFunctions()
{
    std::vector<std::string> fns;
    for (const auto& f : to_robot_func) {
        fns.push_back(f.first);
    }
    return fns;
}

//! 関数が登録済みかどうかを返す(これ要る?)
bool isFuncRegistered(const std::string& func_name)
{
    return !(to_robot_func.find(func_name) == to_robot_func.end());
}

//! 登録された関数名の一覧を表示(これ要る?)
void print_function_list()
{
    const auto fns = getAllRegisteredFunctions();
    std::cout << "functions( " << fns.size() << " ) = ";
    for (const auto& f : fns) {
        std::cout << f << ", ";
    }
    std::cout << std::endl;
}

//! 関数の呼び出し
/*! フロントエンドから関数呼び出しの情報が送られてきたとき呼び出される
 * callback_mutexをロックする
 * \param name 関数名
 * \param args_json 引数(jsonを文字列化したもの)
 * \param cli 呼び出し元クライアントの情報
 */
void callToRobot(const std::string& name, const std::string& args_json, std::ostream* err)
{
    if (!isFuncRegistered(name)) {
        *err << "No such function :" << name << " : args = " << args_json << std::endl;
        print_function_list();
        return;
    }
    *err << "Running" << std::endl;
    try {
        std::lock_guard lock(callback_mutex);
        to_robot_func[name].callback(args_json);
        *err << "Done" << std::endl;
    } catch (const std::exception& e) {
        *err << e.what() << std::endl;
    }
}


//! 変数の値変更の呼び出し
/*! フロントエンドから変数値変更の呼び出しの情報が送られてきたとき呼び出される
 * callback_mutexをロックする
 * \param name 変数名
 * \param args_json 引数(jsonを文字列化したもの)
 * \param cli 呼び出し元クライアントの情報
 */
/*void changeVarToRobot(
    const std::string& name, const std::string& args_json, const std::shared_ptr<Client>& cli)
{
    try {
        std::lock_guard lock(internal_mutex);
        std::lock_guard lock2(callback_mutex);
        to_robot_var[name].callback(args_json);
    } catch (const std::exception& e) {
        cli->err << e.what() << std::endl;
    }
}
*/

//! 現在時刻(epochからのミリ秒)を返す
std::int64_t getTime()
{
    // epochからのミリ秒
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

//! 現在の設定情報をjsonに変換する
std::string settingJson()
{
    Json::Value root(Json::objectValue);
    Json::Value js_ftor(Json::arrayValue), js_svtor(Json::arrayValue), js_svfr(Json::arrayValue),
        js_i(Json::arrayValue), js_cp(Json::arrayValue), js_rs(Json::arrayValue),
        js_gb(Json::arrayValue), js_ga(Json::arrayValue);
    {
        std::lock_guard lock(internal_mutex);
        for (const auto& f : to_robot_func) {
            Json::Value info;
            info["name"] = f.first;
            info["args"] = Json::Value(Json::arrayValue);
            for (std::size_t ai = 0; ai < f.second.names.size(); ai++) {
                Json::Value ainfo(Json::objectValue);
                ainfo["name"] = f.second.names[ai];
                ainfo["type"] = f.second.types[ai].getRepr();
                if (ai < f.second.default_values.size()) {
                    ainfo["default"] = f.second.default_values[ai];
                }
                info["args"].append(ainfo);
            }
            js_ftor.append(info);
        }
        for (const auto& f : to_robot_var) {
            Json::Value info;
            info["name"] = f.first;
            info["value"] = Json::Value(Json::arrayValue);
            for (std::size_t ai = 0; ai < f.second.names.size(); ai++) {
                Json::Value ainfo(Json::objectValue);
                ainfo["name"] = f.second.names[ai];
                ainfo["type"] = f.second.types[ai].getRepr();
                info["value"].append(ainfo);
            }
            js_svtor.append(info);
        }
        for (const auto& f : from_robot) {
            for (std::size_t ai = 0; ai < f.second.names.size(); ai++) {
                Json::Value info;
                if (f.second.names.size() == 1 && f.first == f.second.names[ai]) {
                    info["name"] = f.first;
                } else {
                    info["name"] = f.first + "." + f.second.names[ai];
                }
                info["type"] = f.second.types[ai].getRepr();
                js_svfr.append(info);
            }
        }
        for (const auto& f : images) {
            Json::Value info;
            info["name"] = f.first;
            js_i.append(info);
        }
        for (const auto& rs : related_servers) {
            Json::Value info;
            info["addr"] = rs.addr;
            info["port"] = rs.port;
            js_rs.append(info);
        }
        for (const auto& name : button_name) {
            js_gb.append(name);
        }
        for (const auto& name : axis_name) {
            js_ga.append(name);
        }
    }
    root["server_name"] = getServerName();
    root["version"] = WEBCFACE_VERSION;
    root["functions"] = js_ftor;
    root["to_robot"] = js_svtor;
    root["from_robot"] = js_svfr;
    root["images"] = js_i;
    root["related_servers"] = js_rs;
    root["gamepad_button"] = js_gb;
    root["gamepad_axis"] = js_ga;
    std::stringstream ss;
    ss << root;
    return ss.str();
}

//! 設定情報を全クライアントに送信する
/*! 設定が変更されたとき呼び出される
 */
void updateSetting()
{
    if (clients.size() >= 1) {
        const auto setting_json = settingJson();
        for (const auto& cli : clients) {
            cli.second->send_settings(setting_json);
        }
    }
}
bool setting_changed = false;

std::map<std::string, Json::Value> from_robot_old_values;
//! fromRobotの値をjsonに変換する
/*! \param changed_only trueのとき、前回の呼び出しから変更されていない値はスキップ
 */
std::string fromRobotJson(bool changed_only)
{
    Json::Value root(Json::objectValue);
    {
        std::lock_guard lock(internal_mutex);
        for (const auto& f : from_robot) {
            const auto callback_json = f.second.callback();
            if (!changed_only || from_robot_old_values[f.first] != callback_json) {
                from_robot_old_values[f.first] = callback_json;
                for (std::size_t ai = 0; ai < f.second.names.size(); ai++) {
                    if (f.second.names.size() == 1 && f.first == f.second.names[ai]) {
                        root[f.first] = callback_json;
                    } else {
                        root[f.first + "." + f.second.names[ai]]
                            = callback_json[f.second.names[ai]];
                    }
                }
            }
        }
    }
    root["timestamp"] = static_cast<Json::Value::Int64>(getTime());
    std::stringstream ss;
    ss << root;
    return ss.str();
}

std::map<std::string, Json::Value> layout_old_values;
std::string layoutJson(bool changed_only)
{
    Json::Value root(Json::arrayValue);
    {
        std::lock_guard lock(internal_mutex);

        for (const auto& f : custom_page_layout) {
            Json::Value info;
            info["name"] = f.first;
            info["layout"] = Json::objectValue;
            for (std::size_t ci = 0; ci < f.second.size(); ci++) {
                if (!changed_only
                    || layout_old_values[f.first][static_cast<int>(ci)]
                           != f.second[static_cast<int>(ci)]) {
                    info["layout"][std::to_string(ci)] = f.second[static_cast<int>(ci)];
                }
            }
            info["length"] = f.second.size();
            root.append(info);
            layout_old_values[f.first] = f.second;
        }
    }
    std::stringstream ss;
    ss << root;
    return ss.str();
}

//! 画像の情報をjsonに変換する
/*! \param changed_only trueのとき、前回の呼び出しから変更されていない値はスキップ
 */
std::string imageJson(bool changed_only)
{
    Json::Value root(Json::objectValue);
    {
        std::lock_guard lock(internal_mutex);

        for (auto f : images) {
            if (f.second.has_changed) {
                root[f.first] = f.second.src;
                f.second.has_changed = false;
            } else if (!changed_only) {
                root[f.first] = f.second.src;
            }
        }
    }
    std::stringstream ss;
    ss << root;
    return ss.str();
}

//! logをjsonに変換する
/*! \param changed_only trueのとき、前回の呼び出し以前のログはスキップ
 */
std::string logJson(bool changed_only)
{
    StdLogger::iterator begin, end;
    // ダブルクオーテーションで囲うだけでなくエスケープもしないといけないので、json化
    Json::Value log_json(Json::arrayValue);
    {
        std::lock_guard lock(internal_mutex);

        if (changed_only) {
            begin = std_logger.next();
            end = std_logger.end();
            if (begin == end) {
                return "";
            }
        } else {
            begin = std_logger.begin();
            end = std_logger.current();
        }
        for (auto itr = begin; itr < end; itr++) {
            Json::Value log_line(Json::objectValue);
            log_line["timestamp"] = static_cast<Json::Value::Int64>(itr->timestamp);
            log_line["level"] = itr->level;
            log_line["text"] = itr->text;
            log_json.append(log_line);
        }
    }
    std::stringstream ss;
    ss << log_json;
    return ss.str();
}

//! 値の変更を取得し、全クライアントに送信する
void sendData()
{
    static long last_millisec = 0;
    long now_millisec = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch())
                            .count();

    if (clients.size() == 0) {
        return;
    }
    if (setting_changed) {
        updateSetting();
        setting_changed = false;
    }
    const auto from_robot_json = fromRobotJson(true);
    const auto log = logJson(true);
    for (const auto& cli : clients) {
        cli.second->send_fromRobot(from_robot_json);
    }
    if (now_millisec - last_millisec >= 100) {
        const auto image_json = imageJson(true);
        const auto layout_json = layoutJson(true);
        for (const auto& cli : clients) {
            cli.second->send_image(image_json);
            cli.second->send_layout(layout_json);
        }
        last_millisec = now_millisec;
    }
    if (log != "") {
        for (const auto& cli : clients) {
            cli.second->send_log(log);
        }
    }
}
}  // namespace Server
}  // namespace WebCFace
