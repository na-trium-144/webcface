# Log

\tableofcontents
\since
<span class="since-c"></span>
<span class="since-js"></span>
<span class="since-py"></span>
\sa
* C++ webcface::Log (`webcface/log.h`)
* JavaScript [Log](https://na-trium-144.github.io/webcface-js/classes/Log.html)
* Python [webcface.Log](https://na-trium-144.github.io/webcface-python/webcface.log.html#webcface.log.Log)

テキストのログ出力を送受信します。

trace, debug, info, warning, error, critical の6段階のレベルに分けたログを扱うことができます。
(外部ライブラリのspdlogをそのままインタフェースに利用していたver1の仕様を引き継いでいる)

## コマンドライン
```sh
webcface-send -t log
```
を実行し、文字列を入力すると送信されます。

他のコマンドからpipeしてWebCFaceに送信するという使い方ができます。
詳細は [webcface-send](./71_send.md) のページを参照

## 送信

<div class="tabbed">

- <b class="tab-title">C++</b>
    <span class="since-c">2.0</span>
    Log::append() でログを送信することができます。
    時刻(std::chrono::system_clock)を指定することもできます(省略するとnow()になります)  
    webcfaceに送信されるのみで、コンソールへの出力などは行いません。
    ```cpp
    wcli.log().append(webcface::level::info, "this is info");
    wcli.log().append(webcface::level::error, "this is error");
    ```

    ostream, (<span class="since-c">2.0</span>wostream) を使いたい場合は
    ```cpp
    wcli.loggerOStream() << "hello" << std::endl;
    ```
    のように出力したり、
    ```cpp
    std::cout.rdbuf(wcli.loggerStreamBuf());
    std::cout << "hello" << std::endl;
    ```
    のようにcoutやcerrの出力先を置き換えることができます。
    これらはWebCFaceに出力すると同時に標準エラー出力にも出力します。
    (ver1.11以前はspdlogのstderr_sink、 ver2.0以降はfputs,fputcを使って直接stderrに出力されます)
    またこの場合はログレベルが設定できず、常にinfoになります。
    
    <span class="since-c">2.0</span>
    wostreamを使用したい場合は wcli.loggerWOStream(), wcli.loggerWStreamBuf() を使用するとWebCFaceに出力すると同時にstderrにも出力されます。
    その際Windowsでは出力文字列は Encoding::usingUTF8() の設定に従いUTF-8またはANSIに変換されるため、出力したいコンソールのコードページに設定を合わせてください。
    
- <b class="tab-title">Python</b>
    Python標準のloggingモジュールを使うことができます。

    Client.logging_handler をLoggerのhandlerとして追加して使います。
    ```py
    from logging import getLogger, DEBUG, StreamHandler
    logger = getLogger(__name__)
    logger.setLevel(DEBUG)
    ch = StreamHandler()
    ch.setLevel(DEBUG)
    logger.addHandler(ch)
    logger.addHandler(wcli.logger_handler) # これでloggerのhandlerに登録される

    logger.info("hello") # コンソールとwebcfaceの両方に出力される
    ```

    printを使いたい場合など、 Client.logging_io で取得できるIOオブジェクトを使って送信することもできます。
    logging_io はwebcfaceに送信すると同時に`sys.__stderr__`にも出力します。
    ```py
    import sys
    sys.stdout = wcli.logging_io
    print("hello")
    ```

</div>

### 外部ライブラリの利用

<div class="tabbed">

- <b class="tab-title">spdlog (C++)</b>
  spdlog: https://github.com/gabime/spdlog 

  spdlog→webcfaceにログを送信するsinkの例(ver1.11までwebcfaceに含まれていた実装):
  ```cpp
  class LoggerSink final : public spdlog::sinks::base_sink<std::mutex> {
      webcface::Log *wcli_log;

    protected:
      void sink_it_(const spdlog::details::log_msg &msg) override {
          if (auto *buf_ptr = msg.payload.data()) {
              std::string log_text(buf_ptr, buf_ptr + msg.payload.size());
              if (log_text.size() > 0 && log_text.back() == '\n') {
                  log_text.pop_back();
              }
              if (log_text.size() > 0 && log_text.back() == '\r') {
                  log_text.pop_back();
              }
              wcli_log->append(msg.level, msg.time, log_text);
          }
      }
      void flush_() override {}

    public:
      explicit LoggerSink(webcface::Client &wcli)
      : spdlog::sinks::base_sink<std::mutex>(), wcli_log(wcli.log()) {}
      void set_pattern_(const std::string &) override {}
      void set_formatter_(std::unique_ptr<spdlog::formatter>) override {}
  };
  ```

- <b class="tab-title">log4js (JavaScript)</b>
    log4js: https://www.npmjs.com/package/log4js

    log4jsのappenderにClient.logAppenderを登録すると出力されます。
    ```js
    import log4js from "log4js";

    log4js.configure({
      appenders: {
        out: { type: "stdout" },
        wcli: { type: wcli.logAppender },
      },
      categories: {
        default: { appenders: ["out", "wcli"], level: "debug" },
      },
    });
    const logger = log4js.getLogger();
    logger.info("this is info");
    logger.warn("this is warn");
    logger.error("this is error");
    ```

</div>

<details><summary>spdlogライブラリとの連携(C++ 〜ver1.11)</summary>

Client::logger() はWebCFaceと標準エラー出力に出力するloggerです。
これを使ってログを出力できます。
```cpp
wcli.logger()->info("this is info");
wcli.logger()->error("this is error, error no = {}", 123);
```
これと同様に`trace`, `debug`, `info`, `warn`, `error`, `critical`の6段階のログレベルを使うことができます。

上の例のようにspdlogではfmtライブラリを使用しており数値などをフォーマットして出力することができます。
詳細は[spdlog](https://github.com/gabime/spdlog), [fmt](https://github.com/fmtlib/fmt)を参照してください

Client::loggerSink()でログをwebcfaceに送信するsinkを取得できるので、これを他のloggerにセットして使うこともできます。
または Client.logger() の出力先のsinkを変更することもできます。

</details>

## 受信

Member::log() でLogクラスのオブジェクトが得られ、
Log::tryGet() でデータのリクエストをするとともにログが得られます。

<span class="since-c">2.0</span>
ワイド文字列で取得したい場合は tryGetW() を使います

ログデータは  
C++: webcface::LogLine, webcface::LogLineW  
JavaScript: [LogLine](https://na-trium-144.github.io/webcface-js/interfaces/LogLine.html)  
Python: [webcface.LogLine](https://na-trium-144.github.io/webcface-python/webcface.log_handler.html#webcface.log_handler.LogLine)  
のリストとして得られ、
メッセージ、ログレベル、時刻を取得できます。

<span class="since-c">1.1.5</span>
<span class="since-js">1.0.4</span>
<span class="since-py"></span>
Logオブジェクトにはこれまで受信したログデータがすべて含まれますが、
前回からの差分だけが必要な場合は、データの処理後に Log::clear() で受信したログデータをすべて削除することもできます。

\note
<span class="since-c">1.1.9</span>
サーバーは各クライアントのログを1000行まで保持しています。
logの受信リクエストを送った時点から1000行より前のログは取得できません。
serverの起動時のオプションでこの行数は変更できます。(`webcface-server -h`を参照)

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Func](30_func.md) | |

</div>
