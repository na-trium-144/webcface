# 5-5. Log

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

trace(0), debug(1), info(2), warning(3), error(4), critical(5) の6段階のレベルに分けたログを扱うことができます。
(外部ライブラリのspdlogをそのままインタフェースに利用していたver1の仕様を引き継いでいる)

\note
WebCFaceはログレベルを単に数値として扱うので、-1以下や6以上のレベルも一応使用可能です。

\warning
<span class="since-c">2.4</span><span class="since-js">1.9</span><span class="since-py">2.1</span>
でLogの送受信メッセージを仕様変更しています。
クライアントがこれら以上のバージョンでサーバーが2.3以前の場合Logデータは送受信できません。
(serverのログ出力に `Unknown message kind 8` というwarningが表示されます。)
逆にサーバーが2.4以上でクライアントが古い場合は問題ありません。


## コマンドライン
```sh
webcface-send -t log
```
を実行し、文字列を入力すると送信されます。

他のコマンドからpipeしてWebCFaceに送信するという使い方ができます。
詳細は [webcface-send](./72_send.md) のページを参照

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

    コンソールにも表示しつつwebcfaceにも送信したい場合は、
    std::ostream を使って
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
    (<del>spdlogのstderr_sinkを使って</del> <span class="since-c">2.0</span>fputs,fputcを使って 直接stderrに出力されます)
    またこの場合はログレベルが設定できず、常にinfoになります。
    
    <span class="since-c">2.0</span>
    wostreamを使用したい場合は wcli.loggerWOStream(), wcli.loggerWStreamBuf() を使用するとWebCFaceに出力すると同時にstderrにも出力されます。
    その際Windowsでは出力文字列は usingUTF8() の設定に従いUTF-8またはANSIに変換されるため、出力したいコンソールのコードページに設定を合わせてください。
    
    <span class="since-c">2.4</span>
    Valueなど他のデータ型と同様名前をつけて複数のLogを送信することができます。
    名前を省略した場合、および過去のバージョンから送信されたログデータは `"default"` という名前のLogとして扱われます。
    ```cpp
    wcli.log("hoge").append(webcface::level::info, "this is info");
    wcli.loggerOStream("hoge") << "hello" << std::endl;
    std::cout.rdbuf(wcli.loggerStreamBuf("hoge"));
    ```

- <b class="tab-title">JavaScript</b>
    <span class="since-js">1.8</span>
    Log.append() でログを送信することができます。
    webcfaceに送信されるのみで、コンソールへの出力などは行いません。
    ```ts
    wcli.log().append(2, "this is info");
    wcli.log().append(3, "this is error");
    ```

    <span class="since-js">1.9</span>
    Valueなど他のデータ型と同様名前をつけて複数のLogを送信することができます。
    名前を省略した場合、および過去のバージョンから送信されたログデータは `"default"` という名前のLogとして扱われます。
    ```ts
    wcli.log("hoge").append(2, "this is info");
    ```

- <b class="tab-title">Python</b>
    <span class="since-py">2.0</span>
    Log.append() でログを送信することができます。
    時刻を指定することもできます(省略するとdatetime.datetime.now())になります  
    webcfaceに送信されるのみで、コンソールへの出力などは行いません。
    ```python
    wcli.log().append(2, "this is info")
    wcli.log().append(4, "this is error")
    ```

    コンソールにも表示しつつwebcfaceにも送信したい場合は、
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

    printを使いたい場合は、 Client.logging_io で取得できるIOオブジェクトを使って送信することもできます。
    logging_io はwebcfaceに送信すると同時に`sys.__stderr__`にも出力します。
    ```py
    import sys
    sys.stdout = wcli.logging_io
    print("hello")
    ```

    <span class="since-py">2.1</span>
    Valueなど他のデータ型と同様名前をつけて複数のLogを送信することができます。
    名前を省略した場合、および過去のバージョンから送信されたログデータは `"default"` という名前のLogとして扱われます。
    ```cpp
    wcli.log("hoge").append(2, "this is info");
    ```

</div>

### 外部ライブラリの利用

<div class="tabbed">

- <b class="tab-title">spdlog (C++)</b>
  spdlog: https://github.com/gabime/spdlog 

  spdlog→webcfaceにログを送信するsinkの例(ver1.11までwebcfaceに含まれていた実装):
  ```cpp
  #include <spdlog/sinks/base_sink.h>
  
  class LoggerSink final : public spdlog::sinks::base_sink<std::mutex> {
      webcface::Log wcli_log;

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
              wcli_log.append(msg.level, msg.time, log_text);
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

    log4js→webcfaceにログを送信するappenderの例(ver1.7までwebcfaceに含まれていた実装):
    ```ts
    import { Level, Levels, LoggingEvent, AppenderModule } from "log4js";
    import log4js from "log4js";

    function log4jsLevelConvert(level: Level, levels: Levels) {
      if (level.isGreaterThanOrEqualTo(levels.FATAL)) {
        return 5;
      } else if (level.isGreaterThanOrEqualTo(levels.ERROR)) {
        return 4;
      } else if (level.isGreaterThanOrEqualTo(levels.WARN)) {
        return 3;
      } else if (level.isGreaterThanOrEqualTo(levels.INFO)) {
        return 2;
      } else if (level.isGreaterThanOrEqualTo(levels.DEBUG)) {
        return 1;
      } else if (level.isGreaterThanOrEqualTo(levels.TRACE)) {
        return 0;
      } else {
        return -1;
      }
    }
    export function appender(): AppenderModule {
      return {
        configure:
          (config?: object, layouts?: any, findAppender?: any, levels?: Levels) =>
          (logEvent: LoggingEvent) => {
            wcli.log().append(
              levels !== undefined ? log4jsLevelConvert(logEvent.level, levels) : 2,
              // eslint-disable-next-line @typescript-eslint/no-unsafe-argument
              util.format(...logEvent.data)
            );
          },
      };
    }

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

\note
(サーバーが<span class="since-c">1.1.9</span>以降の場合)
サーバーは各クライアントのログを1000行まで保持しています。
logの受信リクエストを送った時点から1000行より前のログは取得できません。
serverの起動時のオプションでこの行数は変更できます。([2-1. Server](21_server.md)を参照)

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::log() でLogクラスのオブジェクトが得られ、
    Log::tryGet() でデータのリクエストをするとともにログが得られます。

    データは
    webcface::LogLine
    のリストとして得られ、メッセージ、ログレベル、時刻を取得できます。

    ```cpp
    std::optional<std::vector<LogLine>> logs = wcli.member("foo").log().tryGet();
    ```
    * 値をまだ受信していない場合 tryGet() はstd::nulloptを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient::sync()したときに</del>
        <span class="since-c">1.2</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * Log::get() はstd::nulloptの代わりに空のvectorを返します。

    \warning
    <span class="since-c">2.1</span>
    Clientはmemberごとに最大1000行までのログを保持しています。
    それ以上の行数のログがある場合は古いものから削除され、tryGet()やget()で取得できなくなります。  
    保持するログの行数は `webcface::Log::keepLines()` で変更できます。
    負の値にすると無制限に保持するようになります(ver2.0までと同じ動作)

    <span></span>

    <span class="since-c">1.1.5</span>
    Log::tryGet(), get() はClientが保持しているログデータすべてを返しますが、
    前回からの差分だけが必要な場合は、データの処理後に Log::clear() で受信したログデータをすべて削除することもできます。

    ```cpp
    while(true){
        for(LogLine line : wcli.member("foo").log().get()){
            // 新しく送られてきたログについてなにかする
        }
        wcli.member("foo").log().clear(); // 次get()をしたときにはすでに処理済みのログは返ってこない

        // ...
    }
    ```

    <span class="since-c">1.7</span>
    Log::request() で明示的にリクエストを送信することもできます。

    <span class="since-c">2.0</span>
    tryGetW(), getW() ではstringの代わりにwstringを使った webcface::LogLineW のリストで返ります。

    <span class="since-c">2.4</span>
    Valueなど他のデータ型と同様、
    `wcli.member("foo").log("hoge")` のように受信するLogの名前を指定できます。
    名前を省略した場合、および過去のバージョンから送信されたログデータは `"default"` という名前のLogとして扱われます。

- <b class="tab-title">JavaScript</b>
    Member.log() でLogクラスのオブジェクトが得られ、
    Log.tryGet() でデータのリクエストをするとともにログが得られます。

    データは
    [LogLine](https://na-trium-144.github.io/webcface-js/interfaces/LogLine.html)
    のリストとして得られ、メッセージ、ログレベル、時刻を取得できます。

    ```ts
    const logs: LogLine[] | null = wcli.member("foo").log().tryGet();
    ```
    * 値をまだ受信していない場合 tryGet() はnullを返し、そのデータのリクエストをサーバーに送ります。
        * リクエストは <del>次にClient.sync()したときに</del>
        <span class="since-js">1.1</span>自動的に別スレッドで送信されます。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * Log.get() はnullの代わりに空のリストを返します。
    
    \warning
    <span class="since-js">1.8</span>
    Clientはmemberごとに最大1000行までのログを保持しています。
    それ以上の行数のログがある場合は古いものから削除され、tryGet()やget()で取得できなくなります。  
    保持するログの行数は `Log.keepLines = 1000` などとすると変更できます。
    負の値にすると無制限に保持するようになります(ver2.0までと同じ動作)

    <span></span>

    <span class="since-js">1.0.4</span>
    Log.tryGet(), get() はClientが保持しているログデータすべてを返しますが、
    前回からの差分だけが必要な場合は、データの処理後に Log.clear() で受信したログデータをすべて削除することもできます。

    ```ts
    while(true){
        for(const line of wcli.member("foo").log().get()){
            // 新しく送られてきたログについてなにかする
        }
        wcli.member("foo").log().clear(); // 次get()をしたときにはすでに処理済みのログは返ってこない

        // ...
    }
    ```

    <span class="since-js">1.1</span>
    Log::request() で明示的にリクエストを送信することもできます。

    <span class="since-js">1.9</span>
    Valueなど他のデータ型と同様、
    `wcli.member("foo").log("hoge")` のように受信するLogの名前を指定できます。
    名前を省略した場合、および過去のバージョンから送信されたログデータは `"default"` という名前のLogとして扱われます。

- <b class="tab-title">Python</b>
    Member.log() でLogクラスのオブジェクトが得られ、
    Log.try_get() でデータのリクエストをするとともにログが得られます。

    データは
    [webcface.LogLine](https://na-trium-144.github.io/webcface-python/webcface.log_handler.html#webcface.log_handler.LogLine)
    のリストとして得られ、メッセージ、ログレベル、時刻を取得できます。

    ```ts
    logs = wcli.member("foo").log().try_get()
    ```
    * 値をまだ受信していない場合 try_get() はNoneを返し、そのデータのリクエストをサーバーに送ります。
        * そのデータを受信した後([4-1. Client](./41_client.md)を参照)、再度tryGet()することで値が得られます。
    * Log.get() はNoneの代わりに空のリストを返します。
    
    Log.tryGet(), get() はそれまでに受信したログデータすべてを返しますが、
    前回からの差分だけが必要な場合は、データの処理後に Log.clear() で受信したログデータをすべて削除することもできます。

    ```python
    while True:
        for line in wcli.member("foo").log().get():
            # 新しく送られてきたログについてなにかする
        wcli.member("foo").log().clear(); # 次get()をしたときにはすでに処理済みのログは返ってこない
    ```

    Log::request() で明示的にリクエストを送信することもできます。

    <span class="since-py">2.1</span>
    Valueなど他のデータ型と同様、
    `wcli.member("foo").log("hoge")` のように受信するLogの名前を指定できます。
    名前を省略した場合、および過去のバージョンから送信されたログデータは `"default"` という名前のLogとして扱われます。

</div>

### Entry

\since <span class="since-c">2.1</span><span class="since-js">1.8</span><span class="since-py">2.0</span>

(サーバーが<span class="since-c">2.1</span>以降の場合のみ)

ログをすべて受信しなくても、ログが少なくとも1行存在するかどうか(他memberが送信しているかどうか)は取得することができます。

Log.exists()
はログが少なくとも1行存在する場合、trueを返します。
tryGet() と違い、ログデータそのものを受信するリクエストは送られません。

<span class="since-c">2.4</span>
<span class="since-js">1.9</span>
<span class="since-py">2.1</span>
Valueなどの他の型と同様、 Memberが送信しているLogのリストを取得したり、
LogEntry イベントも使えるようになりました。

詳細は [4-3. Field](./43_field.md) を参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [5-4. View](54_view.md) | [6-1. Canvas2D](61_canvas2d.md) |

</div>
