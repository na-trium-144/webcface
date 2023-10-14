## [1.1.1] - 2023-10-14
### Fixed
* brewでインストールしたspdlog,fmtを使ったときのビルドエラーを修正 (#45)

## [1.1.0] - 2023-10-14
### Fixed
* 処理が遅くても通りやすくなるようテストを修正 (#42)
### Changed
* webcface-jsに合わせてViewのメッセージの形式を変更 (#41)
	* webcface-1.0とはメッセージの互換性がありません
* MSVC, MinGWでビルドできるようにした (#36)
* updated dependencies

## [1.0.2] - 2023-10-06
### Fixed
* client再接続後にデータを受信できなくなるバグ (= pingが機能しないことがあるバグ #13) の修正 (#33)

### Changed
* server側でping関連のdebug,traceメッセージ追加 (#33)
* ping_status_req受信時にすぐping_statusを送り返すようにした (#33)
* updated dependencies

## [1.0.1] - 2023-10-03
### Added
* Documentationを書きました (#10, #28)
* MacOSでのテストを追加 (#3)

### Fixed
* armv7でのコンパイル時のwarningを修正 (#9)

### Changed
* Value::hidden, Text::hidden, View::hidden を削除 (#10)
* ViewComponentのViewComponentBaseからの継承をprotectedにした (#10)
* Client::logger_sink, logger_ostream, logger_streambuf をそれぞれloggerSink, loggerOStream, loggerStreamBufに変更した (#28)

## [1.0.0] - 2023-09-27
