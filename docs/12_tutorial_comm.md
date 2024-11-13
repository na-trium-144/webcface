# 1-2. Tutorial (Communication)

\tableofcontents

WebCFaceの機能紹介・チュートリアルです。

このチュートリアルはプロセス間通信をメインにしたものです。
WebUIでのデータの可視化やWebUIからのプログラム操作に重点をおいたチュートリアルは
[1-1. Tutorial (Visualizing)](./11_tutorial_vis.md)

このチュートリアルでは C++ (Meson または CMake)、または Python (Python3.6以上が必要です) を使用します。

## WebCFaceのインストール

READMEにしたがって webcface, webcface-webui, webcface-tools をインストールしましょう。

C++の場合はインストールしたwebcfaceパッケージにクライアントライブラリも含まれています。

## Server

WebCFaceを使用するときはserverを常時立ち上げておく必要があります。
次のように WebCFace Desktop アプリから、またはコマンドラインからの2つの方法があります。

<div class="tabbed">

- <b class="tab-title">WebCFace Desktop</b>

    スタートメニュー(Windows)、
    アプリケーションメニュー(Ubuntu)、
    Launchpadまたはアプリケーションフォルダ(Mac)
    に
    「WebCFace Desktop」のアイコン
    <img src="https://raw.githubusercontent.com/na-trium-144/webcface-webui/main/public/icon.svg" height="24" />
    が表示されているはずなので、それを起動してください。

    それを起動すると、WebCFaceのメイン画面(WebUI)が起動すると同時に、バックグラウンドでサーバーが起動します。

- <b class="tab-title">コマンドライン</b>

    ターミナルを開いて `webcface-server` コマンドを実行するとサーバーが起動します。
    デフォルトでは7530番ポートを開きクライアントの接続を待ちます。
    追加で指定できるコマンドラインオプションなど、詳細は [2-1. Server](./21_server.md)

</div>

## Setup

ここからWebCFaceを使ったプログラムを書いていきます。

まずはプロジェクトを作成しWebCFaceライブラリを使えるようにします。

\note
C++でMesonやCMakeを使わない場合、pkg-configを使ったり手動でコンパイル・リンクすることも可能です。
詳細な説明は [3-1. Setup WebCFace Library](./31_setup.md)

<div class="tabbed">

- <b class="tab-title">C++ (Meson)</b>
    適当にディレクトリを作ります
    ```sh
    mkdir webcface-tutorial
    cd webcface-tutorial
    ```
    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * send.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h> // ← webcface::Client

    webcface::Client wcli("tutorial-send");

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (sender)" << std::endl;
    }
    ```
    * recv.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h>

    webcface::Client wcli("tutorial-recv");  // ← ここの名前が違うことに注意

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (receiver)" << std::endl;
    }
    ```
    \note
    Clientの初期化はグローバル変数でもローカル変数 (main() の最初) でもどちらでも構いませんが、
    このチュートリアルではグローバル変数にします。

    * meson.build
    ```meson
    project('webcface-tutorial', 'cpp',
      default_options: ['cpp_std=c++17'],
    )
    webcface_dep = dependency('webcface')
    executable('tutorial-send', 'send.cc',
      dependencies: webcface_dep,
    )
    executable('tutorial-recv', 'recv.cc',
      dependencies: webcface_dep,
    )
    ```
    
    ビルドして、実行します
    ```sh
    meson setup build  # ← 初回のみ
    meson compile -C build
    ./build/tutorial-send
    ```
    ターミナルをもう1つ開いて
    ```sh
    ./build/tutorial-recv
    ```
    実行すると、`Hello, World! (sender)` `Hello, World! (receiver)` と出力されます。

    \note
    このチュートリアルでは以降ビルドと実行の手順は省略しますが、
    同様に `meson compile` (またはninja) でビルドして `./build/tutorial-send` と `./build/tutorial-recv` を実行してください。

    <span></span>

- <b class="tab-title">C++ (CMake)</b>
    適当にディレクトリを作ります
    ```sh
    mkdir webcface-tutorial
    cd webcface-tutorial
    ```
    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * send.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h> // ← webcface::Client

    webcface::Client wcli("tutorial-send");

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (sender)" << std::endl;
    }
    ```
    * recv.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h>

    webcface::Client wcli("tutorial-recv");  // ← ここの名前が違うことに注意

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (receiver)" << std::endl;
    }
    ```
    \note
    Clientの初期化はグローバル変数でもローカル変数 (main() の最初) でもどちらでも構いませんが、
    このチュートリアルではグローバル変数にします。
    
    * CMakeLists.txt
    ```cmake
    cmake_minimum_required(VERSION 3.5)
    project(webcface-tutorial)
    find_package(webcface 2.0 CONFIG REQUIRED)
    add_executable(tutorial-send send.cc)
    target_link_libraries(tutorial-send PRIVATE webcface::webcface)
    add_executable(tutorial-recv recv.cc)
    target_link_libraries(tutorial-recv PRIVATE webcface::webcface)
    ```
    
    ビルドして、実行します
    ```sh
    cmake -B build  # ← 初回のみ
    cmake --build build
    ./build/tutorial-send
    ```
    ターミナルをもう1つ開いて
    ```sh
    ./build/tutorial-recv
    ```
    実行すると、`Hello, World! (sender)` `Hello, World! (receiver)` と出力されます。

    \note
    このチュートリアルでは以降ビルドと実行の手順は省略しますが、
    同様に `cmake --build` (またはmakeやninjaなどでも可) でビルドして `./build/tutorial-send` と `./build/tutorial-recv` を実行してください。

    <span></span>

- <b class="tab-title">Python</b>
    PythonでWebCFaceを使うには、クライアントライブラリをインストールする必要があります。
    venv内でもグローバルにインストールしても構いません。
    ```sh
    pip install webcface
    ```

    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * send.py
    ```py
    from webcface import Client

    wcli = Client("tutorial-send")
    wcli.wait_connection()

    print("Hello, World! (sender)")
    ```
    * recv.py
    ```py
    from webcface import Client

    wcli = Client("tutorial-recv")  # ←ここの名前が違うことに注意
    wcli.wait_connection()

    print("Hello, World! (receiver)")
    ```
    \note
    Clientの初期化はグローバル変数でもローカル変数 (main() やクラスの初期化など) でもどちらでも構いませんが、
    このチュートリアルではグローバル変数にします。
    
    実行します
    ```sh
    python3 ./send.py
    ```
    ターミナルをもう1つ開いて
    ```sh
    python3 ./recv.py
    ```
    実行すると、`Hello, World! (sender)` `Hello, World! (receiver)` と出力されます。

</div>

## Value, Text

まずは数値や文字列のデータを送受信してみましょう。
(他のPub-Sub型通信でいうところのTopicをPublishするようなものです)

\note
以降このチュートリアルではC++同士・Python同士の通信だけでなく、C++のsend側とPythonのrecv側、のように組み合わせても動作します。

<div class="tabbed">

- <b class="tab-title">C++</b>

    * send.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h>
    #include <webcface/value.h> // ← value() を使うのに必要
    #include <webcface/text.h> // ← text() を使うのに必要

    webcface::Client wcli("tutorial-send");

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (sender)" << std::endl;

        // "data" という名前のValueデータとして 100 という値をセット
        wcli.value("data") = 100;
        // "message" という名前のTextデータとして "Hello, World! (sender)" という文字列をセット
        wcli.text("message") = "Hello, World! (sender)";
        wcli.sync(); // データを送信します

        std::cout << "sender finish" << std::endl;
    }
    ```
    * recv.cc
    ```cpp
    #include <iostream>
    #include <thread>
    #include <webcface/client.h>
    #include <webcface/value.h> // ← value() を使うのに必要
    #include <webcface/text.h> // ← text() を使うのに必要

    webcface::Client wcli("tutorial-recv");

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (receiver)" << std::endl;

        while (true) {
            // "tutorial-send" が送信している "data" という名前のValueデータをリクエスト & 取得
            std::optional<double> data = wcli.member("tutorial-send").value("data").tryGet();
            if (data.has_value()){
                // 送信されたデータがあれば取得できるはず
                std::cout << "data = " << *data << std::endl;
            } else {
                // まだ何も送信されていない
                std::cout << "data is null" << std::endl;
            }

            // "tutorial-send" が送信している "message" という名前のTextデータをリクエスト & 取得
            std::optional<std::string> message = wcli.member("tutorial-send").text("message").tryGet();
            if (message.has_value()){
                std::cout << "message = " << *message << std::endl;
            } else {
                std::cout << "message is null" << std::endl;
            }

            wcli.sync(); // データを受信します
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    実行すると、send側で送信した `data` と `message` の内容が、1秒ごとにrecv側にも表示されます。
    起動する順番はどちらが先でも大丈夫です。
    ```
    Hello, World! (receiver)
    data is null
    message is null
    data = 100
    message = Hello, World! (sender)
    data = 100
    message = Hello, World! (sender)
    ```

    \note
    * recv側は最初からsend側が送信したすべてのデータを受信しているわけではなく、
    1回目のtryGet()呼び出しで `data` と `message` の内容をリクエストしています。
        * (この時点では値はnulloptになっています)
        * そしてその後の `wcli.sync()` (受信したデータの処理が行われる) をした後の2回目のtryGet()の呼び出しで実際に値が得られます。
    * recv側を先に起動した場合、send側が起動するまでの間はデータはnulloptのままになります。
    * send側とrecv側を1度実行して終了した後に再度recv側だけを起動すると、send側を起動しなくても値を取得できてしまいますが、
    これはサーバーのほうにデータが残っているためです。
        * サーバーも1度終了して起動し直すとまた値がない状態に戻ります。

    <span></span>

- <b class="tab-title">Python</b>

    * send.py
    ```py
    from webcface import Client

    wcli = Client("tutorial-send")
    wcli.wait_connection()

    print("Hello, World! (sender)")

    # "data" という名前のvalueデータとして100という値をセット
    wcli.value("data").set(100)
    # "message" という名前のtextデータとして "Hello World! (sender)" という文字列をセット
    wcli.text("message").set("Hello, World! (sender)")

    wcli.sync()  # データを送信します
    print("sender finish")
    ```
    * recv.py
    ```py
    from webcface import Client
    import time

    wcli = Client("tutorial-recv")
    wcli.wait_connection()

    print("Hello, World! (receiver)")

    while True:
        # "tutorial-send" が送信している "data" という名前のvalueデータをリクエスト&取得
        data = wcli.member("tutorial-send").value("data").try_get()
        if data is not None:
            # 送信されたデータがあれば取得できるはず
            print(f"data = {data}")
        else:
            # まだなにも送信されていない
            print("data is None")

        # "tutorial-send" が送信している "message" という名前のtextデータをリクエスト&取得
        message = wcli.member("tutorial-send").text("message").try_get()
        if message is not None:
            print(f"message = {message}")
        else:
            print("message is None")

        wcli.sync()  # データを受信します
        time.sleep(1)
    ```

    実行すると、send側で送信した `data` と `message` の内容が、1秒ごとにrecv側にも表示されます。
    起動する順番はどちらが先でも大丈夫です。
    ```
    Hello, World! (receiver)
    data is None
    message is None
    data = 100
    message = Hello, World! (sender)
    data = 100
    message = Hello, World! (sender)
    ```

    \note
    * recv側は最初からsend側が送信したすべてのデータを受信しているわけではなく、
    1回目のtry_get()呼び出しで `data` と `message` の内容をリクエストしています。
        * (この時点では値はNoneになっています)
        * そしてその後の `wcli.sync()` (受信したデータの処理が行われる) をした後の2回目のtry_get()の呼び出しで実際に値が得られます。
    * recv側を先に起動した場合、send側が起動するまでの間はデータはNoneのままになります。
    * send側とrecv側を1度実行して終了した後に再度recv側だけを起動すると、send側を起動しなくても値を取得できてしまいますが、
    これはサーバーのほうにデータが残っているためです。
        * サーバーも1度終了して起動し直すとまた値がない状態に戻ります。

    <span></span>

</div>

## コールバック

データを受信する側では、現在の値を繰り返し確認するのではなく、値が変化した時に処理が実行されるようにするという使い方もできます。
(他のPub-Sub型通信でいうところのSubscribeのようなものです)

<div class="tabbed">

- <b class="tab-title">C++</b>

    * send側は上のプログラムと同じです。
    * recv.cc
    ```cpp
    #include <iostream>
    #include <thread>
    #include <webcface/client.h>
    #include <webcface/value.h>
    #include <webcface/text.h>

    webcface::Client wcli("tutorial-recv");

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (receiver)" << std::endl;

        // ちなみに "tutorial-send" を毎回書くのは面倒なのでこのように変数に入れることもできます
        webcface::Member sender = wcli.member("tutorial-send");

        // "tutorial-send" が送信している "data" という名前のValueデータが変わった時に呼び出される処理
        sender.value("data").onChange([](webcface::Value v){
            std::cout << "data changed: " << v.get() << std::endl;
        });

        // "tutorial-send" が送信している "message" という名前のTextデータが変わった時に呼び出される処理
        sender.text("message").onChange([](webcface::Text t){
            std::cout << "message changed: " << t.get() << std::endl;
        });

        while (true) {
            wcli.sync(); // データを受信し、値が変化すればコールバックが呼ばれます
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    これを実行すると、1秒ごとに受信した値が表示される代わりに、send側からデータが送られてきたタイミングで1度だけ
    ```
    data changed: 100
    message changed: Hello, World! (sender)
    ```
    のように表示されます。

    このプログラムではsend側は常に同じデータを送っていますが、違うデータを送るようにすればデータが変わったタイミングで再度コールバックが呼ばれます。

    \note
    このサンプルプログラムではsend側がデータを送ってからrecv側のコールバックが呼ばれるまで1秒のラグがあると思います。これはrecv側がデータを受信する処理(`wcli.sync()`)が1秒に1回しか呼ばれていないためです。
    これを改善するにはsync()を呼ぶ頻度を上げるか、
    [4-1. Client](41_client.md) のページで説明していますが`wcli.loopSync()`などが使えます。

    Value, Textについては [5-1. Value](./51_value.md), [5-2. Text](52_text.md) に詳細なドキュメントがあります。

- <b class="tab-title">Python</b>

    * send側は上のプログラムと同じです。
    * recv.py
    ```py
    from webcface import Client, Value, Text
    import time

    wcli = Client("tutorial-recv")
    wcli.wait_connection()

    print("Hello, World! (receiver)")

    # ちなみに "tutorial-send" を毎回書くのは面倒なのでこのように変数に入れることもできます
    sender = wcli.member("tutorial-send")

    # tutorial-send が送信している "data" という名前のValueデータが変わったときに呼び出される処理
    @sender.value("data").on_change
    def on_data_change(v: Value):
        print(f"data changed: {v.get()}")

    # tutorial-send が送信している "message" という名前のTextデータが変わったときに呼び出される処理
    @sender.text("message").on_change
    def on_message_change(t: Text):
        print(f"message changed: {t}")

    while True:
        wcli.sync()  # データを受信し、値が変化すればコールバックが呼ばれます
        time.sleep(1)
    ```
    \note
    `@sender.value("data").on_change` はデコレータです(糖衣構文というPythonの構文です)。
    この書き方に馴染みがないなら
    ```py
    def on_data_change(v: Value):
        print(f"data changed: {v.get()}")

    sender.value("data").on_change(on_data_change)
    ```
    と書いても同じです(がこの場合は@を使ったほうがかんたんに書けます)。

    <span></span>

    これを実行すると、1秒ごとに受信した値が表示される代わりに、send側からデータが送られてきたタイミングで1度だけ
    ```
    data changed: 100
    message changed: Hello, World! (sender)
    ```
    のように表示されます。

    このプログラムではsend側は常に同じデータを送っていますが、違うデータを送るようにすればデータが変わったタイミングで再度コールバックが呼ばれます。

    \note
    このサンプルプログラムではsend側がデータを送ってからrecv側のコールバックが呼ばれるまで1秒のラグがあると思います。これはrecv側がデータを受信する処理(`wcli.sync()`)が1秒に1回しか呼ばれていないためです。
    これを改善するにはsync()を呼ぶ頻度を上げるか、
    [4-1. Client](41_client.md) のページで説明していますが`wcli.sync(timeout)`などが使えます。

    Value, Textについては [5-1. Value](./51_value.md), [5-2. Text](52_text.md) に詳細なドキュメントがあります。


</div>

## Func

データを送信するだけでなく、プロセス間で関数を呼び出すこともできます。
このチュートリアルではsend側の関数をrecv側から呼び出します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    * send.cc で引数なしの関数hogeを作り、これをクライアントに登録します。
    ```cpp
    #include <webcface/func.h> // ←追加
    #include <thread>

    int hoge() {
        std::cout << "Function hoge started" << std::endl;
        return 42;
    }

    int main() {
        wcli.func("hoge").set(hoge);

        // 省略...

        // 関数が呼び出されるのを待つために、無限ループを追加します
        while (true){
            wcli.sync();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```
    * recv.cc では1秒おきにそれを呼び出します。
    ```cpp
    #include <webcface/func.h> // ← 追加

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (receiver)" << std::endl;

        webcface::Member sender = wcli.member("tutorial-send");

        // 省略...

        while (true) {
            // tutorial-send の "hoge" という関数を呼び出します
            webcface::Promise hoge_p = sender.func("hoge").runAsync();
            hoge_p.onFinish([hoge_p](){
                // hoge_pが完了したとき、結果を表示します
                if(hoge_p.isError()){
                    std::cout << "Error in hoge(): " << hoge_p.rejection() << std::endl;
                }else{
                    std::cout << "hoge() = " << hoge_p.response().asInt() << std::endl;
                }
            });

            wcli.sync(); // データを受信します
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    実行すると、send側とrecv側が両方起動していればsend側では1秒おきに hoge() が呼び出され(`Function hoge started`と表示され)、recv側ではhoge()の戻り値である42が表示されるはずです。

    send側が起動していない状態でrecv側だけを起動すると呼び出しは失敗し、エラー表示になります。
    またsend側の関数の中でthrowをした場合にもエラーになります。

    \note
    * 関数の呼び出しに1秒程度ラグがあると思います。
    これは関数を実際に実行する処理が wcli.sync() (このチュートリアルでは1秒に1回呼んでいる) の中で行われているためです。
    これを改善するにはsync()を呼ぶ頻度を上げるか、
    [4-1. Client](41_client.md) のページで説明していますが`wcli.loopSync()`などが使えます。
    * また、send側では呼び出された関数の実行が完了するまで wcli.sync() は完了しません。
    メインループをブロックせず別スレッドで関数を呼び出したい場合は set() の代わりに setAsync() が使えます。

    <span></span>

    こんどは引数がある関数を作ってみます。

    * send.cc
    ```cpp
    int fuga(int a, const std::string &b) {
        logger << "Function fuga(" << a << ", " << b << ") started" << std::endl;
        return a;
    }

    int main() {
        wcli.func("hoge").set(hoge);
        wcli.func("fuga").set(fuga);

        // 以下略...
    }
    ```
    * recv.cc
    ```cpp
    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World! (receiver)" << std::endl;

        webcface::Member sender = wcli.member("tutorial-send");

        // 省略...

        while (true) {
            // tutorial-send の "hoge" という関数を呼び出します
            webcface::Promise hoge_p = sender.func("hoge").runAsync();
            hoge_p.onFinish([hoge_p](){
                // hoge_pが完了したとき、結果を表示します
                if(hoge_p.isError()){
                    std::cout << "Error in hoge(): " << hoge_p.rejection() << std::endl;
                }else{
                    std::cout << "hoge() = " << hoge_p.response().asInt() << std::endl;
                }
            });

            // tutorial-send の "fuga" という関数を引数を渡して呼び出します
            webcface::Promise fuga_p = sender.func("fuga").runAsync(123, "abc");
            fuga_p.onFinish([fuga_p](){
                // fuga_pが完了したとき、結果を表示します
                if(fuga_p.isError()){
                    std::cout << "Error in fuga(123, abc): " << fuga_p.rejection() << std::endl;
                }else{
                    std::cout << "fuga(123, abc) = " << fuga_p.response().asInt() << std::endl;
                }
            });

            wcli.sync(); // データを受信します
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    引数は整数型、実数型、bool、文字列型であればいくつでも使うことができます。
    呼び出す側ではrunAsync()に引数を渡せば、引数のない関数と同様に呼び出すことができます。

    Funcについては [5-3. Func](./53_func.md) に詳細なドキュメントがあります。

- <b class="tab-title">Python</b>
    * send.cc で引数なしの関数hogeを作り、これをクライアントに登録します。
    ```py
    from webcface import Client
    import time

    wcli = Client("tutorial-send")
    
    # 関数を"hoge"という名前のFuncとして登録
    @wcli.func("hoge")
    def hoge() -> int:
        print("Function hoge started")
        return 42

    # デコレータを使わないなら wcli.func("hoge").set(hoge)

    wcli.wait_connection()

    # 省略...

    # 関数が呼び出されるのを待つために、無限ループを追加します
    while True:
        wcli.sync()
        time.sleep(1)
    ```
    * recv.py では1秒おきにそれを呼び出します。
    ```py
    from webcface import Client, Promise
    import time

    wcli = Client("tutorial-recv")
    wcli.wait_connection()

    print("Hello, World! (receiver)")

    sender = wcli.member("tutorial-send")

    # 省略...

    while True:
        # tutorial-send の "hoge" という関数を呼び出します
        hoge_p = sender.func("hoge").run_async()
        @hoge_p.on_finish
        def on_hoge_finish(hoge_p: Promise):
            # hoge_p が完了したとき、結果を表示します
            if hoge_p.is_error:
                print(f"Error in hoge(): {hoge_p.rejection}")
            else:
                print(f"hoge() = {hoge_p.response()}")

        # デコレータを使わないなら
        # hoge_p.on_finish(on_hoge_finish)

        wcli.sync() # データを受信します
        time.sleep(1)
    ```

    実行すると、send側とrecv側が両方起動していればsend側では1秒おきに hoge() が呼び出され(`Function hoge started`と表示され)、recv側ではhoge()の戻り値である42が表示されるはずです。

    send側が起動していない状態でrecv側だけを起動すると呼び出しは失敗し、エラー表示になります。
    またsend側の関数の中で例外をraiseした場合にもエラーになります。

    \note
    * 関数の呼び出しに1秒程度ラグがあると思います。
    これは関数を実際に実行する処理が wcli.sync() (このチュートリアルでは1秒に1回呼んでいる) の中で行われているためです。
    これを改善するにはsync()を呼ぶ頻度を上げるか、
    [4-1. Client](41_client.md) のページで説明していますが`wcli.sync(timeout)`などが使えます。
    * また、send側では呼び出された関数の実行が完了するまで wcli.sync() は完了しません。
    メインループをブロックせず別スレッドで関数を呼び出したい場合は set() の代わりに set_async() が使えます。

    <span></span>

    こんどは引数がある関数を作ってみます。

    * send.py
    ```py
    from webcface import Client

    wcli = Client("tutorial-send")

    @wcli.func("fuga")
    def fuga(a: int, b: str) -> int:
        print(f"Function fuga({a}, {b}) started")
        return a

    wcli.wait_connection()
    # 以下略...
    ```
    * recv.py
    ```py
    from webcface import Client, Promise
    import time

    wcli = Client("tutorial-recv")
    wcli.wait_connection()

    print("Hello, World! (receiver)")

    sender = wcli.member("tutorial-send")

    # 省略...

    while True:
        # tutorial-send の "hoge" という関数を呼び出します
        hoge_p = sender.func("hoge").run_async()
        @hoge_p.on_finish
        def on_hoge_finish(hoge_p: Promise):
            # hoge_p が完了したとき、結果を表示します
            if hoge_p.is_error:
                print(f"Error in hoge(): {hoge_p.rejection}")
            else:
                print(f"hoge() = {hoge_p.response()}")

        # tutorial-send の "fuga" という関数を引数を渡して呼び出します
        fuga_p = sender.func("fuga").run_async(123, "abc")
        @fuga_p.on_finish
        def on_fuga_finish(fuga_p: Promise):
            # fuga_p が完了したとき、結果を表示します
            if fuga_p.is_error:
                print(f"Error in fuga(): {fuga_p.rejection}")
            else:
                print(f"fuga() = {fuga_p.response()}")

        # デコレータを使わないなら
        # fuga_p.on_finish(on_fuga_finish)

        wcli.sync() # データを受信します
        time.sleep(1)
    ```
    
    引数は整数型、実数型、bool、文字列型であればいくつでも使うことができます。
    呼び出す側ではrunAsync()に引数を渡せば、引数のない関数と同様に呼び出すことができます。

    Funcについては [5-3. Func](./53_func.md) に詳細なドキュメントがあります。

</div>

## おわりに

以上で 1-2. Tutorial (Communication) は終わりです。

ここで紹介していない機能として
* [6-2. Image](62_image.md)

も送受信することができるデータとしてあるので、見てみてください。

また、チュートリアルでは紹介していないコマンドラインツールとして
* [7-2. webcface-send](72_send.md)

もあります。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [1-1. Tutorial (Visualizing)](11_tutorial_vis.md) | [2-1. Server](21_server.md) |

</div>
