## [1.2.0] - 2023-12-07
### Changed
* ABIバージョンをlibwebcface.so.2 に変更。～v1.1とは互換性がありません
	* `WebCFace::ClientData`を`webcface::Internal::ClientData`に変更 (#79)
	* `SyncDataStore2`からhidden属性を削除し、`FuncInfo`のメンバーに追加
	* `WebCFace::stderr_sink`と`WebCFace::logger_internal_level` を削除
	* namespaceを`WebCFace`から`webcface`に変更
		* 従来の`WebCFace`もaliasとして使用可能
* 各種データのリクエストは`Client::sync()`を待たずに送信されるように変更 (#100)
* loggerの出力したデータが即時`Client::log()`に反映されるよう変更
* `View`のコピーやムーブで内部の`ViewBuf`を共有するように変更 (#99)
* main以外のブランチのPRでもテストが実行されるようにした

### Added
* `Client::start()`, `Client::waitConnection()` (#100)
* `webcface::Version::version`, `webcface::Version::version_s` (#99)
* LICENSEファイルが /usr/share/webcface 以下にインストールされるようにした
* gitから自分でビルドした場合、バージョン番号に`git describe`の情報がサフィックスとして追加されるようにした
* `cpack -G NSIS`でwindows用のインストーラーを生成できるようにした
* colcon(ament_cmake)でビルド可能にした

### Fixed
* デフォルト構築したViewがデストラクタで例外を投げるバグを修正 (#79)
* `onSync`イベントの発生タイミングを受信データの処理後に変更 (#99)

## [1.1.9] - 2023-11-30
### Fixed
* 1024バイト以上の文字列を受信可能にした (#111)
* unpack時にエラーが発生した場合にcatchするようにした (#112)
### Changed
* libcurlの不要な機能を無効化 (#101)
### Added
* namespaceを小文字の`webcface`にしても使用できるようにした (#106)
* サーバーが保持するログを直近1000行に制限し、それを変更するオプションを追加 (#105, #110)
* Readmeにライセンス表記を追加

## [1.1.8] - 2023-11-20
### Fixed
* brewでインストールしたspdlog,fmtを使ったときのビルドエラーを修正 (#97)

## [1.1.7] - 2023-11-20
### Changed
* クライアントのwebsocketライブラリをlibcurlに、サーバー側をcrowに変更 (#90)
* webuiのURLを http://ipアドレス:7530/index.html に変更
* WebCFace::stderr_sink と WebCFace::logger_internal_level をdeprecatedにした
### Added
* 環境変数`WEBCFACE_VERBOSE`, `WEBCFACE_TRACE`でデバッグ情報を表示できるようにした
* サーバー側でデバッグ情報を表示できる`-v`オプション追加
### Fixed
* RunCondOnSyncでデッドロックすることがあるバグを修正

## [1.1.6] - 2023-11-18
### Fixed
* MacOSでbrew installするとexampleが動かないのを修正 (#89)

## [1.1.5] - 2023-11-18
### Changed
* readmeとtutorialを改訂 (#67)
* viewのViewBufがflushされるタイミングを変更
	* View::operator<< をoverrideされてないoperator<<と同じ動作にした
* submoduleのwebuiを削除
* リポジトリ直下にdist/を置いても認識するようにした
* updated dependencies
### Added
* Log::clear() (#67)

## [1.1.4] - 2023-10-27
### Fixed
* spdlog v1.5.0でビルドが通るよう修正(それより前は未検証) (#66)
### Changed
* Releaseのdebパッケージがubuntuのspdlogパッケージを使うようにした
* 依存ライブラリの都合でciでのReleaseビルドをubuntu-22.04に戻した

## [1.1.3] - 2023-10-25
### Added
* staticライブラリをビルドできるCMakeオプション追加 (#64)
### Fixed
* MinGWでセグフォする問題を修正? (#53)
	* 根本的な解決ではない上、メモリリークが増加
* CMakeでwebcface_test_timeoutを省略するとエラーになるのを修正 (#53)
* messageのunpackでエラーが起きた時デフォルト値を返しエラーになるのを修正 (#63)
### Changed
* Linux Clang, MinGW GCCのテストをciに追加 (#53)
* serverのヘルプにデフォルトポートの表示を追加 (#56)
* ciでのテストとReleaseビルドをubuntu-22.04から20.04に変更 (#64)
* updated dependencies

## [1.1.2] - 2023-10-19
### Added
* ドキュメントにチュートリアル追加 (#46 #49)
* exampleがserverといっしょにインストールされるようにした (#52)
### Changed
* testで通信のタイムアウト時間を変更できるようにした (#50)
* コマンドラインパーサーをtclapからcli11に変更 (#51)
* updated dependencies

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
