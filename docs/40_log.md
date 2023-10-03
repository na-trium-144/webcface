# Log

API Reference → WebCFace::Log

テキストのログ出力を送受信します。

## ログを出力する

Client::logger() でspdlogのloggerを取得でき、これを使ってログを出力できます
```cpp
wcli.logger()->info("this is info");
wcli.logger()->error("this is error, error no = {}", 123);
```
これと同様に`trace`, `debug`, `info`, `warn`, `error`, `critical`の6段階のログレベルを使うことができます。

上の例のようにspdlogではfmtライブラリを使用しており数値などをフォーマットして出力することができます。
詳細は[spdlog](https://github.com/gabime/spdlog), [fmt](https://github.com/fmtlib/fmt)を参照してください

また、Client::loggerSink() でspdlogのsinkを取得できます。

spdlogではなくostreamを使いたい場合は、Client::loggerOStream() や Client::loggerStreamBuf() を使えます。
この場合はログレベルが常にinfoになります。

## ログを取得する

Member::log() でLogクラスのオブジェクトが得られます
```cpp
WebCFace::Log log_a = wcli.member("a").log();
```

Log::tryGet() でログデータが得られます。
```cpp
std::nullopt<std::vector<WebCFace::LogLine>> log = log_a.tryGet();
```
Log::get() はstd::nulloptの代わりにデフォルト値を返す点以外は同じです。

## 受信イベント

Log::appendListener() などで受信したデータが変化したときにコールバックを呼び出すことができます
```cpp
wcli.member("a").log().appendListener([](Log v){ /* ... */ });
```
