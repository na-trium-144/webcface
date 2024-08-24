## [2.0.0] - 2024-08-24
### Changed
* ABIバージョン: 20
* ビルドシステムをMesonに移行 (#361)
	* FetchContentからwrapに移行
	* インストール済みの外部ライブラリも使用可能になった
	* warning_levelをデフォルトで3に設定
	* webcface-server が実行時にwebcfaceライブラリに依存しないようにした (#358)
	* MinGWでDebugビルドがリンクエラーにならないようになった
	* windowsでdllのバージョン番号は `webcface-20.dll` のようになり、libにはバージョン番号がつかないようになった
	* MinGWではDebugビルドにpostfixをつけない (#301)
	* Cygwinでビルド可能になった(クライアントのみ)
	* ver0のCMakeファイルが読み込まれているときのエラーメッセージ削除 (#368)
	* staticビルドができるようになった (#274)
* webcface/common/ ディレクトリの削除, ディレクトリ構造の見直し (#283)
* clang-tidyの導入、warning修正 (#256)
* C++17に移行 (#329)
* eventpp依存を削除 (#311)
* API, ABIのインタフェースでのspdlog依存を削除 (#316)
* 画像処理周りをOpenCVからImageMagickに移行 (#270, #301)
	* ImageFrameとcv::Matの相互変換機能削除
	* ImageBaseをdeprecatedにした
	* ImageBaseWithCV 削除
	* Image::mat() 削除
* 受信処理をClient::sync()を呼んだスレッドで行うようにした (#310, #357, #367)
	* Func::set()でセットした関数はrecv()と同じスレッドでそのまま呼ばれるように仕様変更
		* Func::runCond... 系の機能を削除
		* その代わりとして Func::setAsync() 追加
	* Client::loopSync(), loopSyncFor(), loopSyncUntil() 追加
	* wcfWaitConnection, wcfAutoConnection, wcfLoopSync, wcfLoopSyncFor, wcfLoopSyncUntil, wcfFuncSetAsync, wcfFuncSetAsyncW 追加
	* クライアントが内部で建てるスレッドを1つのみにした
	* Client::close() がsync()で送信したデータの送信完了を待機するようにした
	* AsyncFuncResult → Promise
		* Promise::started, result をdeprecatedにした
		* Promise::reached, found, onReach, waitReach, waitReachFor, waitReachUntil, finished, response, rejection, rejectionW, onFinish, waitFinish, waitFinishFor, waitFinishUntil 追加
	* FuncCallHandle → CallHandle
	* wcfAsyncFuncResult → wcfPromise
		* wcfDestroy が wcfPromise ポインタを受け付けるようにした
	* Func::run と operator(), wcfFuncRun, wcfFuncRunW をdeprecatedにした
	* wcfFuncSetで登録した関数がrespondせずにreturnした場合に自動でrespondしないようにした
	* サーバーに接続していない間各種リクエストや関数呼び出しを送らないようにした
		* Func::runAsync()時に未接続なら即座に呼び出しは失敗するようになった
* messageのSvrVersionをSyncInitEndに改名、member id と hostname のフィールドを追加 (#325, #349)
	* SyncInitEndはEntryがすべて終わってから送られてくるようにした
	* Client::onPing(), pingStatus() がClient自身にも使えるようになった
	* Client::waitConnection() はSyncInitEndが完了するまで待機するようにした
	* Client::serverHostName() 追加
* ABI
	* Linux,Macで -fvisibility=hidden を指定し、webcface以外のライブラリのシンボルを隠す (#274)
		* 実行時に依存ライブラリが一切不要になった
	* MinGWでdllexport, dllimportを使うようにした (#274)
	* すべての関数に `__cdecl` 指定を追加 (#333)
	* MacOSでdylibのcompatibility_versionを設定 (#368)
	* namespaceにabiバージョンを含める (#368)
	* ほとんどのクラスのPimpl化 (#373)
* その他のAPI変更
	* namespaceをsnake_caseに統一 (#312, #317)
	* ValAdaptor::asDouble, asInt, asLLong関数を追加し、テンプレートのas()をdeprecatedにした (#283)
	* ViewComponentBase → ViewComponent, TemporalViewComponent (#283, #373)
	* Canvas2DComponentBase → Canvas2DComponent, TemporalCanvas2DComponent
	* Canvas3DComponentBase → Canvas3DComponent, TemporalCanvas3DComponent
	* Dict, VectorOpt 削除
	* RobotJoint, RobotLink のメンバー変数削除、getter関数にした (#373)
	* Canvas2D, Canvas3D のdeprecatedだったadd関数を削除 (#373)
	* Text をTextとVariantに分離(#375)
		* ViewComponent::bind() の戻り値をVariant型に変更
	* InputRef::get() の仕様変更
	* LogLineData<> のメンバーアクセスをgetter関数に変更 (#279)
	* Func::set() などで渡した関数オブジェクトはコピーではなく常にムーブするようにした (#310)
	* wcfTextGet, wcfValueGet が配列の代わりにNULLを受け付けられるようにした (#346)
	* wcfValueGetが値の受信してない場合返す例外をwcfNotFound→wcfNoDataに変更
	* 大半のメンバ関数にconst追加 (#374)
	* View << callback のときcallbackの引数にView&が使えなくなった
### Added
* utf-8とstring, wstringの相互変換機能 (#264, #279, #370)
	* 各種クラスの関数でstringを扱うものについてwstringバージョンを追加
	* Text::tryGet() の戻り値をstd::stringに変更
	* Text::get() の戻り値をstd::stringのconst参照に変更 (#375)
	* Text::tryGetW(), getW() 追加
	* usingUTF8(), wcfUsingUTF8() を追加
	* メンバー名、フィールド名が無効なutf-8の場合サーバー側で置き換えるようにした
* CのAPI強化 (#346)
	* wcfMemberList/W, wcfMemberEntryEvent/W, wcfServerVersion, wcfServerName, wcf\*EntryList/W, wcf\*EntryEvent/W, wcfMemberSyncEvent/W, wcfMemberLibName, wcfMemberLibVersion, wcfRemoteAddr, wcfValueChangeEvent/W 追加
	* enum wcfStatus, enum wcfValType, enum wcfViewComponentType, enum wcfColor 追加、define定数削除
* Examples
	* ImageMagickを使った画像送受信 example-image-send, example-image-recv (#270)
	* OpenCVを使った画像送受信 example-cv-send, example-cv-recv
### Fixed
* Transformのデフォルトコンストラクタのバグを修正 (#283)
* クライアントがサーバーに接続直後同じデータが2重で送信されるバグを修正 (#367)

## [1.11.4] - 2024-06-12
### Changed
* CMakeListsでfetchするcrowのタグを固定 (#277)

## [1.11.3] - 2024-05-03
### Changed
* def.hとversion.rcの生成をbuildディレクトリ内に変更 (#259)

## [1.11.2] - 2024-05-03
### Fixed
* MacOSでValAdaptorのコンパイルエラー修正 (#255)
* Windowsでcanvas_dataのコンパイルエラー修正 (#257)

## [1.11.1] - 2024-04-30
### Added
* Client::autoReconnect() (#254)
### Fixed
* clientがwebsocket通信をするスレッドがCPUを100%使っていた問題を改善 (#254)

## [1.11.0] - 2024-04-26
### Added
* Ubuntu24.04ビルドのReleaseを追加 (#246)
* 利用可能な場合Unixドメインソケットで通信するようにした (#233)
* WSL2とWindowsの相互接続が可能 (WSL2ではlocalhostの代わりにWindowsホストにも接続を試行するようにした) (#233)
* Value,Textなど各種Fieldを==で比較する機能、ViewComponent,Canvas2DComponentなどを==で比較する機能 (#234)
* Clientのインタフェース改良 (#241)
	* Field::child, operator[], parent, lastName 追加
	* Value,Textなど各種Fieldに対しても child, operator[], parent, lastName 追加
	* Member::value, text, valueEntries, textEntries ...etcをField::に移動
	* Value::set, get でparentの配列の要素を参照する機能追加
	* Value::resize, push_back 追加
	* EventTarget::callbackList() 追加
	* InputRef::asStringRef, asString, as<>, asBool 追加
	* ValAdaptor::empty, InputRef::empty 追加
	* View::operator<<(function) 追加
	* AsyncFuncResult::onStarted, onResult 追加
### Changed
* OpenCVのライブラリすべてではなく必要なもの(core,imgcodecs,imgproc)だけをリンクするようにした (#243)
* message, server-implをlibwebcfaceに統合 (#242)
* cmake時にconceptsヘッダーが存在するか確認するのを追加 (#242)
* Value::Dictをdeprecatedにした (#241)
* イベントのコールバックの引数型変更 (#241)
* InputRef::get の戻り値をconst参照にした (#241)
### Fixed
* MinGWでメッセージの受信に15msかかっていたのを修正 (#245)

## [1.10.0] - 2024-04-01
### Added
* viewにinput要素を追加 (#219)
	* ViewComponentType::text_input, decimal_input, number_input, select_input, toggle_input, slider_input, check_input
	* Components::textInput, decimalInput, numberInput, selectInput, toggleInput, sliderInput, checkInput
	* ViewComponent::bind(), onChange(), init(), min(), max(), step(), option()
	* InputRefクラス
* ViewComponent::id() (#227)
### Changed
* Textの内部データ形式をstd::stringからValAdaptorに変更 (#219)
* ValAdaptorのコンストラクタをexplicitにした
* Text::Dict削除
* ピリオドではじまるfieldのentryを送信しない仕様に変更
	* AnonymousFunc, Viewなど内部で使用する名前はピリオド2つで始まるようにした
* windowsのDebugでのnamespaceをwebcfacedからwebcface::debugに変更 (#221)
	* webcface.hをincludeすることなく一部のファイル(client.hとvalue.hだけ、など)のincludeだけでも使えるようになる
* AnonymousFuncのコンストラクタをexplicitにした (#222)
* ViewBufクラスにfinal追加、EventTarget::onAppendのoverrideにfinal追加 (#228)
* ValAdaptorの内部でデータをすべてstringに変換するのをやめた (#229)

## [1.9.1] - 2024-03-23
### Changed
* Func引数で文字列→bool型の変換の仕様を変更 (#217)
### Fixed
* Func呼び出し時引数と戻り値がすべて文字列型に置き換わってしまうのを修正 (#217)

## [1.9.0] - 2024-03-16
### Added
* Canvas2DにonClickとtext要素を追加 (#210)
* TemporalComponentクラスを追加
* wcfFuncCallback型, wcfFuncSet関数 (#213)
* Func::setで引数にhandleをとるcallbackを登録する機能を追加
### Changed
* Geometriesの各関数の戻り値の仕様変更 (#210)
* ViewComponents → Components
* Viewのinit(),add(),sync()の処理を抽象化→ Internal::DataSetBuffer
* Canvas2D,Canvas3D,RobotModelのadd関数仕様変更
* (Windows) クラス全体をdllexportするように変更
* AnonymousFuncオブジェクトをコピー不可にした
* wcfFuncRespondでnullptrを使えるようにした (#213)

## [1.8.0] - 2024-03-08
### Changed
* Value, Textで値が変化したときのみ送信するようにした (#209)
	* Funcは2度送信しないようにした
* Valueをstd::ostreamに出力するときの処理を修正
	* 配列データの表示を追加、未受信はnullと表示するようにした
* server, clientともにメッセージのwarning表示を1回までにした

## [1.7.0] - 2024-02-26
### Added
* Cの関数追加 (#198)
	* wcfValueGet
	* wcfTextSet, wcfTextSetN, wcfTextGet
	* wcfViewComponents, wcfViewSet, wcfViewGet
	* wcfDestroy
* Value::request() など (#199)
* 各種イベントのコールバックに引数を持たない関数を登録可能にした
### Changed
* Funcの実行結果のデータの保持方法を変更 (#198)
	* Client内部にAsyncFuncResultを保持しないようにした
	* 内部で使用している一部メンバ関数の削除、変更
* Client::member() に空文字列が渡された場合thisを返すようにした
* Value::set() にテンプレート引数で std::vector<double> 以外の配列型が使えるようにした (#199)
* Value::time() などを Member::syncTime() に変更

## [1.6.3] - 2024-02-22
### Added
* pkgconfigのファイルを生成するようにした (#200)

## [1.6.2] - 2024-02-18
### Changed
* ver.0の古いWebCFaceのCMakeが読み込まれた時ヘッダーがエラーを出すようにした (#195)
### Fixed
* WindowsでBUILD_TYPEがRelWithDebInfoやMinSizeRelでfind_packageした場合正しくリンクされるよう修正 (#193)
* Windowsでpdbファイルもインストールされるようにした

## [1.6.1] - 2024-02-15
### Fixed
* Canvas2D::add の引数を修正

## [1.6.0] - 2024-02-15
### Added
* Canvas2D (#189)
* Member::func(const T &func) (#185)
### Changed
* ドキュメントを全体的に書き直し。 (#185)
* values → valueEntries など名前変更 (以前の関数はdeprecatedになります) (#185)

## [1.5.3] - 2024-02-09
### Fixed
* Macでserver終了時にabortするバグを修正 (#184)
### Changed
* サーバー側でtext, log, view, funcinfo, call, callresult内の文字列に対してutf-8でデコードできないデータを置き換えるようにした (#183)
### Added
* ReleaseするUbuntuのdebパッケージで lib/systemd/system/webcface-server.service をインストールするようにした (#186)

## [1.5.2] - 2024-01-17
### Fixed
* client側で受信データが途中で途切れてエラーになるバグを修正 (#177)
* 画像変換でデッドロックするのを修正 (#179)
* 画像をリサイズさせると縦横サイズが逆になるバグを修正 (#179)
* サーバー終了時のせぐふぉを修正 (#179)

## [1.5.1] - 2024-01-17
### Fixed
* CMakeでwebcface::webcfaceからc_std_99を削除 (#175)
	* それとは別にC言語のターゲットとしてwebcface::wcfを追加
### Changed
* Crowのinfoレベルのログを非表示にした (#172)
### Added
* serverでlocalhostではなくpcのipアドレスを表示するようにした (#172)

## [1.5.0] - 2024-01-16
### Changed
* ABIバージョンを libwebcface.so.5, webcface5.dll に変更。
* cmake時にfind_packageで読み込んだmsgpack,eventpp,spdlogのバージョンなどの情報を表示するようにした (#171)
### Added
* C言語からwebcfaceにアクセスできる関数を追加 (#136)
* FuncListener追加 (#136)
* 依存ライブラリのmsgpack,eventpp,spdlogについてfind_packageをしないようにするcmakeオプション追加 (#171)

## [1.4.1] - 2024-01-15
### Fixed
* (主にwindows)インストール後にfind_packageしたときインストール前のOpenCV_INCLUDE_DIRのパスを参照してしまっていたのを修正 (#169)

## [1.4.0] - 2024-01-10
### Added
* RobotModel, Canvas3Dの送受信機能追加 (#162)
### Changed
* MinGWのビルドでもdllにバージョン情報が追加されるようにした (#166)
* webcface-serverとexampleにもバージョン情報追加
* dllのファイル名にsoversionを追加 (webcface.dll → webcface3.dll, libwebcface.dll → libwebcface3.dll)
* crowを crowcpp/crow のmasterブランチに変更 (#163)

## [1.3.1] - 2023-12-30
### Changed
* clang17でのコンパイルエラーを修正
### Fixed
* v1.2.2, v1.3.0 でサーバーがsegmentation faultするバグを修正 (#160)

## [1.3.0] - 2023-12-26
### Changed
* ABIバージョンをlibwebcface.so.3 に変更
* debugビルドのdllファイル名をwebcfacedに変更 (#148)
* debugビルドのnamespaceをwebcfacedに変更
	* webcface/webcface.h をインクルードすればwebcfaceにエイリアスが貼られる
* windowsでdllにバージョン情報を追加
### Added
* imageの送受信機能追加(#124)
	* Imageのメッセージ(kind=5)、Imageフィールドクラスを追加
	* s_ClientDataに画像の変換をするスレッドを追加
	* SyncDataStore2に
		* テンプレートパラメータReqTを追加
		* SyncDataStore2::getReqInfoを追加
		* SyncDataStore2::clearRecvを追加
	* opencvへの依存
### Fixed
* -Werrorフラグを設定、clangとgccでのwarningを修正 (#146)
* Memberクラス全体をexportするのをやめた (warningの修正) (#148)
* EventTargetのvirtualデストラクタを追加 (#144)


## [1.2.2] - 2023-12-19
### Changed
* windowsで文字セットがcp932のままwebcfaceをincludeしてもコンパイルが通るようにした (#131)
* Wall, Wextra, Wpedantic (gcc, clang), W3 (msvc) フラグを追加 (#138)
* ciにclang-tidyのチェックを追加
### Fixed
* callで呼び出したメンバーが通信切断されているとき、また呼び出し中に切断されたときに、サーバーがcallresponseやcallresultを呼び出し元に返すようにした (#141)

## [1.2.1] - 2023-12-12
### Changed
* 依存ライブラリをsubmoduleではなくFetchContentで取得するように変更 (#128)
* 依存ライブラリをaptやbrewから取得した場合のビルドもciでテストするようにした (#127)

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
* Ubuntu20.04のReleaseビルドを追加

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
