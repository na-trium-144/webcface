# Log

API Reference → webcface::Log

テキストのログ出力を送受信します。

## ログを出力する

### C++
[spdlog](https://github.com/gabime/spdlog) ライブラリを利用しています。

Client::logger() でspdlogのloggerを取得でき、これを使ってログを出力できます
```cpp
wcli.logger()->info("this is info");
wcli.logger()->error("this is error, error no = {}", 123);
```
これと同様に`trace`, `debug`, `info`, `warn`, `error`, `critical`の6段階のログレベルを使うことができます。

上の例のようにspdlogではfmtライブラリを使用しており数値などをフォーマットして出力することができます。
詳細は[spdlog](https://github.com/gabime/spdlog), [fmt](https://github.com/fmtlib/fmt)を参照してください

Client::logger() はwebcfaceに送信するだけでなく標準出力にも出力します。
この挙動を変えたい場合はlogger()->sinks()を変更するか、
またClient::loggerSink()でログをwebcfaceに送信するsinkを取得できるのでこれを他のloggerにセットして使うこともできます。

spdlogではなくostreamを使いたい場合は
```cpp
wcli.loggerOStream() << "hello" << std::endl;
```
のように出力したり、
```cpp
std::cout.rdbuf(&wcli.loggerStreamBuf());
std::cout << "hello" << std::endl;
```
のようにcoutやcerrの出力先を置き換えることができます。
この場合はログレベルが設定できず、常にinfoになります。

### Python
Python標準のloggingモジュールを使います。

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

また、printを使いたい場合などは Client.logging_io で取得できるIOオブジェクトが使えます。
```py
import sys
sys.stdout = wcli.logging_io
print("hello")
```
### JavaScript
[log4js](https://www.npmjs.com/package/log4js)を使います。

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

## ログを取得する

Member::log() でLogクラスのオブジェクトが得られます
```cpp
webcface::Log log_a = wcli.member("a").log();
```

Log::tryGet() でログデータが得られます。
Valueなどと同様、初回の呼び出しではstd::nulloptを返し、2回目以降はログデータを取得できます。
```cpp
std::nullopt<std::vector<webcface::LogLine>> log = log_a.tryGet();
```
Log::get() はstd::nulloptの代わりに空のリストを返す点以外は同じです。

![c++ ver1.1.9](https://img.shields.io/badge/1.1.9~-00599c?logo=C%2B%2B)
サーバーは各クライアントのログを1000行まで保持しています。
logの受信リクエストを送った時点から1000行より前のログは取得できません。
serverの起動時のオプションでこの行数は変更できます。(`webcface-server -h`を参照)

## 受信イベント

Log::appendListener() で受信したデータが変化したときにコールバックを呼び出すことができます
```cpp
wcli.member("a").log().appendListener([](Log v){ /* ... */ });
```
(Pythonでは Log.signal)

Logオブジェクトにはこれまで受信したログデータがすべて含まれます。  
![c++ ver1.1.5](https://img.shields.io/badge/1.1.5~-00599c?logo=C%2B%2B)
![js ver1.0.4](https://img.shields.io/badge/1.0.4~-f7df1e?logo=JavaScript&logoColor=black)
![py ver1.0](https://img.shields.io/badge/1.0~-3776ab?logo=python&logoColor=white)
前回からの差分だけが必要な場合は、データの処理後に Log::clear() で受信したログデータをすべて削除することもできます。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Func](30_func.md) | |

</div>
