# 1-1. Tutorial (Visualizing)

\tableofcontents

WebCFaceの機能紹介・チュートリアルです。

このチュートリアルはWebUIでデータを可視化する・WebUIからプログラムを操作するという部分をメインにしたものです。
プロセス間通信に重点をおいたチュートリアルは [1-2. Tutorial (Communication)](./12_tutorial_comm.md)

このチュートリアルでは C++ (Meson または CMake)、または Python (Python3.8以上が必要です) を使用します。

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

    serverは起動したまま、起動時に表示されるurl (http://IPアドレス:7530/index.html) をブラウザで開くと、
    WebCFaceのメイン画面(WebUI)が開きます。
    使用しているターミナルによっては、Ctrl(Command)+クリックなどで開くことができるかもしれません。

</div>

## WebUI

WebCFaceのメイン画面です。
WebCFaceにクライアントが接続すると、WebUI右上のMenuに表示されます。
Menuから見たいデータを選ぶことで小さいウィンドウのようなものが現れデータを見ることができます。

ウィンドウの表示状態などは自動的にブラウザ(LocalStorage)に保存され、次回アクセスしたときに復元されます。

詳細な使い方については [2-2. WebUI](./22_webui.md)

![desktop_empty](https://github.com/na-trium-144/webcface/raw/main/docs/images/desktop_empty.png)

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
    
    * main.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h> // ← webcface::Client

    webcface::Client wcli("tutorial");

    int main() {
        wcli.waitConnection();

        std::cout << "Hello, World!" << std::endl;
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
    executable('tutorial', 'main.cc',
      dependencies: webcface_dep,
    )
    ```
    
    ビルドして、実行します
    ```sh
    meson setup build  # ← 初回のみ
    meson compile -C build
    ./build/tutorial
    ```
    実行すると、`Hello, World!` と出力されると同時に、
    WebUI の右上のメニューの中にClientで指定した名前「tutorial」が現れるはずです。

    ![tutorial_helloworld](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_helloworld.png)

    \note
    このチュートリアルでは以降ビルドと実行の手順は省略しますが、
    同様に `meson compile` (またはninja) でビルドして `./build/tutorial` を実行してください。

    <span></span>

- <b class="tab-title">C++ (CMake)</b>
    適当にディレクトリを作ります
    ```sh
    mkdir webcface-tutorial
    cd webcface-tutorial
    ```
    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * main.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h> // ← webcface::Client

    webcface::Client wcli("tutorial");

    int main() {
        wcli.waitConnection(); // serverに接続できるまで待機

        std::cout << "Hello, World!" << std::endl;
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
    add_executable(tutorial main.cc)
    target_link_libraries(tutorial PRIVATE webcface::webcface)
    ```
    
    ビルドして、実行します
    ```sh
    cmake -B build  # ← 初回のみ
    cmake --build build
    ./build/tutorial
    ```
    実行すると、`Hello, World!` と出力されると同時に、
    WebUI の右上のメニューの中にClientで指定した名前「tutorial」が現れるはずです。

    ![tutorial_helloworld](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_helloworld.png)

    \note
    このチュートリアルでは以降ビルドと実行の手順は省略しますが、
    同様に `cmake --build` (またはmakeやninjaなどでも可) でビルドして `./build/tutorial` を実行してください。

    <span></span>

- <b class="tab-title">Python</b>
    PythonでWebCFaceを使うには、クライアントライブラリをインストールする必要があります。
    venv内でもグローバルにインストールしても構いません。
    ```sh
    pip install webcface
    ```

    以下のようにWebCFaceの初期化 + 簡単な Hello World を書きます
    
    * main.py
    ```py
    from webcface import Client

    wcli = Client("tutorial")
    wcli.wait_connection()

    print("Hello, World!")
    ```
    \note
    Clientの初期化はグローバル変数でもローカル変数 (main() やクラスの初期化など) でもどちらでも構いませんが、
    このチュートリアルではグローバル変数にします。
    
    実行します
    ```sh
    python3 ./main.py
    ```
    実行すると、`Hello, World!` と出力されると同時に、
    WebUI の右上のメニューの中にClientで指定した名前「tutorial」が現れるはずです。

    ![tutorial_helloworld](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_helloworld.png)

</div>

## Log

コンソールに出力していた `Hello, World!` を、WebCFaceにも送信してみましょう。

<div class="tabbed">

- <b class="tab-title">C++</b>

    * main.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h>

    webcface::Client wcli("tutorial");
    // loggerOStream() は std::cout と同様に文字列を出力して使うことができる
    std::ostream &logger = wcli.loggerOStream();

    int main() {
        wcli.waitConnection();

        logger << "Hello, World!" << std::endl;

        wcli.sync(); // wcli に書き込んだデータを送信する
    }
    ```

    これを実行すると、コンソール (std::coutではなくstd::cerrと同じ標準エラー出力ですが) には今まで通り `Hello, World!` と表示されます。

    それに加えて、WebUI右上のメニューから「tutorial」を開き「Logs」をクリックすると、ログを表示する画面が開きこちらからも `Hello, World!` を確認できるはずです。

    ![tutorial_log](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_log.png)

    std::wostream を使うこともできます。
    また、コンソールに表示せずWebCFaceにログの文字列を送信する関数もあります。
    詳細は [5-5. Log](./55_log.md)

    <span></span>

- <b class="tab-title">Python</b>

    * main.py
    ```py
    from webcface import Client
    import sys

    wcli = Client("tutorial")
    sys.stdout = wcli.logging_io  # sys.stdout (printの出力先) をwebcfaceのlogging_ioに置き換える
    wcli.wait_connection()

    print("Hello, World!")
    wcli.sync();  # wcli に書き込んだデータを送信する
    ```

    これを実行すると、コンソール (stdoutではなくstderrですが) には今まで通り `Hello, World!` と表示されます。

    それに加えて、WebUI右上のメニューから「tutorial」を開き「Logs」をクリックすると、ログを表示する画面が開きこちらからも `Hello, World!` を確認できるはずです。

    ![tutorial_log](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_log.png)

    pythonのloggingモジュールを使いたい場合はLoggerの出力先として使用できるHandlerも用意しています。
    また、コンソールに表示せずWebCFaceにログの文字列を送信する関数もあります。
    詳細は [5-5. Log](./55_log.md)

</div>

## Value, Text

数値や文字列のデータを送信すると、それをリアルタイムに表示することができます。

<div class="tabbed">

- <b class="tab-title">C++</b>

    * main.cc
    ```cpp
    #include <iostream>
    #include <thread>
    #include <webcface/client.h>
    #include <webcface/value.h> // ← value() を使うのに必要
    #include <webcface/text.h> // ← text() を使うのに必要

    webcface::Client wcli("tutorial");
    std::ostream &logger = wcli.loggerOStream();

    int main() {
        wcli.waitConnection();

        logger << "Hello, World!" << std::endl;

        int i = 0;

        while(true){
            i++;
            wcli.value("hoge") = i; // 「hoge」という名前のvalueに値をセット
            if(i % 2 == 0){
                wcli.text("fuga") = "even"; // 「fuga」という名前のtextに文字列をセット
            }else{
                wcli.text("fuga") = "odd";
            }

            wcli.sync(); // wcli に書き込んだデータを送信する (繰り返し呼ぶ必要がある)
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    これを実行し、WebUI右上のメニューから「tutorial」を開き「hoge」をクリックするとグラフが表示され、リアルタイムにtestの値(ここでは1秒ごとに1ずつ増える値)を確認できます。

    また、「Text Variables」をクリックすると文字列で送信したデータもリアルタイムに確認することができます。

    ![tutorial_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_value.png)

    Value, Textについては [5-1. Value](./51_value.md), [5-2. Text](52_text.md) に詳細なドキュメントがあります。

- <b class="tab-title">Python</b>

    * main.py
    ```py
    from webcface import Client
    import sys
    import time

    wcli = Client("tutorial")
    sys.stdout = wcli.logging_io
    wcli.wait_connection()

    print("Hello, World!")

    i = 0

    while True:
        i += 1
        wcli.value("hoge").set(i)  # 「hoge」という名前のvalueに値をセット
        if i % 2 == 0:
            wcli.text("fuga").set("even")  # 「fuga」という名前のtextに文字列をセット
        else:
            wcli.text("fuga").set("odd")

        wcli.sync()  # wcli に書き込んだデータを送信する (繰り返し呼ぶ必要がある)
        time.sleep(1)
    ```

    これを実行し、WebUI右上のメニューから「tutorial」を開き「hoge」をクリックするとグラフが表示され、リアルタイムにtestの値(ここでは1秒ごとに1ずつ増える値)を確認できます。

    また、「Text Variables」をクリックすると文字列で送信したデータもリアルタイムに確認することができます。

    ![tutorial_value](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_value.png)

    Value, Textについては [5-1. Value](./51_value.md), [5-2. Text](52_text.md) に詳細なドキュメントがあります。

</div>

## Func

プログラムからWebUIに情報を送信するだけでなく、WebUIからプログラムを操作することも可能です。

<div class="tabbed">

- <b class="tab-title">C++</b>
    * 引数なしの関数hogeを作り、これをクライアントに登録します。
    ```cpp
    #include <webcface/func.h> // ←追加

    int hoge() {
        logger << "Function hoge started" << std::endl;
        return 42;
    }

    int main() {
        wcli.func("hoge").set(hoge);

        // 以下略...
    }
    ```

    これを実行し、WebUI右上のメニューから「tutorial」を開き「Functions」をクリックすると hoge() を実行するボタンが現れると思います。
    「Run」をクリックすると実行され、「Function hoge started」のログが追加されます。
    また、画面右下に関数の戻り値の42が表示されています。

    ![tutorial_func1](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_func1.png)

    \note
    Runを押してから「Function hoge started」が表示されるまでに1秒程度ラグがあると思います。
    これは関数を実際に実行する処理が wcli.sync() (このチュートリアルでは1秒に1回呼んでいる) の中で行われているためです。

    <span></span>

    * こんどは引数がある関数を作ってみます。
    ```cpp
    int fuga(int a, const std::string &b) {
        logger << "Function fuga(" << a << ", " << b << ") started" << std::endl;
        return a;
    }

    int main() {
        wcli.func("hoge").set(hoge);
        wcli.func("fuga").set(fuga).setArgs({
            webcface::Arg("a").init(100), // 1つ目の引数の名前はa, 初期値が100
            webcface::Arg("b").option({"foo", "bar", "baz"}), // 2つ目の引数の名前はb, 選択肢がfoo,bar,baz
        });

        // 以下略...
    }
    ```

    setArgs() はそれぞれの引数の名前やオプション(初期値、最小値、最大値、選択肢など)を指定することができます。
    (指定しなくてもよいです。)

    実行すると fuga() の引数を入力する欄と実行するボタンが現れると思います。
    hogeの場合と同様、引数を入力して「Run」をクリックすると実行されます。

    ![tutorial_func2](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_func2.png)

    Funcについては [5-3. Func](./53_func.md) に詳細なドキュメントがあります。

- <b class="tab-title">Python</b>
    * 引数なしの関数hogeを作り、これをクライアントに登録します。
    ```py
    from webcface import Client
    import sys
    import time

    def hoge() -> int:
        print("Function hoge started")
        return 42

    wcli = Client("tutorial")
    wcli.func("hoge").set(hoge)  # 関数hogeを"hoge"という名前のFuncとして登録
    sys.stdout = wcli.logging_io
    wcli.wait_connection()

    # 以下略...
    ```

    これを実行し、WebUI右上のメニューから「tutorial」を開き「Functions」をクリックすると hoge() を実行するボタンが現れると思います。
    「Run」をクリックすると実行され、「Function hoge started」のログが追加されます。
    また、画面右下に関数の戻り値の42が表示されています。

    ![tutorial_func1](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_func1.png)

    \note
    * Runを押してから「Function hoge started」が表示されるまでに1秒程度ラグがあると思います。
    これは関数を実際に実行する処理が wcli.sync() (このチュートリアルでは1秒に1回呼んでいる) の中で行われているためです。
    * また、send側では呼び出された関数の実行が完了するまで wcli.sync() は完了しません。
    メインループをブロックせず別スレッドで関数を呼び出したい場合は set() の代わりに setAsync() が使えます。

    <span></span>

    * こんどは引数がある関数を作ってみます。
    ```py
    from webcface import Client, Arg  # <= Arg のimportを追加
    import sys
    import time

    def hoge() -> int:
        print("Function hoge started")
        return 42

    # webcfaceは型アノテーションを使って引数の型を判別できるので、引数の型は書いたほうがいいです
    def fuga(a: int, b: str) -> int:
        print(f"Function fuga({a}, {b}) started")
        return a

    wcli = Client("tutorial")
    wcli.func("hoge").set(hoge)
    wcli.func("fuga").set(fuga, args=[
        Arg(init=100),  # 1つ目の引数aは初期値が100
        Arg(option=["foo", "bar", "baz"]),  # 2つ目の引数bは選択肢がfoo,bar,baz
    ])
    sys.stdout = wcli.logging_io
    wcli.wait_connection()

    # 以下略...
    ```

    args ではそれぞれの引数の名前やオプション(初期値、最小値、最大値、選択肢など)を指定することができます。
    (指定しなくてもよいです。)

    実行すると fuga() の引数を入力する欄と実行するボタンが現れると思います。
    hogeの場合と同様、引数を入力して「Run」をクリックすると実行されます。

    ![tutorial_func2](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_func2.png)

    Funcについては [5-3. Func](./53_func.md) に詳細なドキュメントがあります。

</div>

## View

Funcに登録した関数は一覧表示させることしかできませんが、
Viewではテキストや入力欄を任意に並べて表示させることができます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    #include <webcface/view.h> // ←追加

    int main() {
        // 省略

        while(true){
            // 他のデータの送信は省略...

            webcface::View v = wcli.view("sample");
            v << "Hello, world!" << std::endl; // テキスト表示
            v << "i = " << i << std::endl;
            v << webcface::button("hoge", hoge) << std::endl; // ボタン
            static webcface::InputRef ref_str;
            v << webcface::textInput("str").bind(ref_str); // 文字列入力
            v << webcface::button("print", []{
                // クリックすると、入力した文字列を表示
                logger << "str = " << ref_str.asString() << std::endl;
              });
            v << std::endl;

            v.sync(); // ここまでにvに追加したものをwcliに反映

            wcli.sync();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    ```

    これを実行し、WebUI右上のメニューから「tutorial」を開き「sample」をクリックすると、
    画像のようにプログラムで指定したとおりの画面が表示されます。
    「hoge」ボタンをクリックすると関数hoge (Function hoge started) と表示し、
    「print」ボタンをクリックするとその左の入力欄に入力した文字列がログに表示されます。

    ![tutorial_view](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

    上のプログラム例のように、
    テキストの表示はstd::ostream (std::cout など) と同じようにviewに文字列や数値などを出力するような書き方でできます。
    ボタンにはFuncと同様関数や関数オブジェクト(ラムダ式など)を登録できます。
    またtextInputなどを使って入力欄を表示させることもできます。
    詳細は [5-4. View](./54_view.md) を参照

- <b class="tab-title">Python</b>
    ```py
    from webcface import Client, Arg, view_components, InputRef

    while True:
        # 他のデータの送信は省略...

        with wcli.view("sample") as v:
            v.add("Hello, world!")  # テキスト表示
            v.add("i = ", i)
            # ↑ v.add("i = ") と v.add(i) をするのと同じ
            v.add(view_components.button("hoge", hoge))  # ボタン
            ref_str = InputRef()
            v.add(view_components.text_input("str", bind=ref_str))  # 文字列入力
            v.add(
                view_components.button(
                    "print",
                    # クリックすると、入力した文字列を表示
                    lambda: print(f"str = {str(ref_str.get())}"),
                )
            )
            v.add("\n")
        # withを抜けると、ここまでにvに追加したものがwcliに反映される

        wcli.sync()  # wcli に書き込んだデータを送信する (繰り返し呼ぶ必要がある)
        time.sleep(1)
    ```

    これを実行し、WebUI右上のメニューから「tutorial」を開き「sample」をクリックすると、
    画像のようにプログラムで指定したとおりの画面が表示されます。
    「hoge」ボタンをクリックすると関数hoge (Function hoge started) と表示し、
    「print」ボタンをクリックするとその左の入力欄に入力した文字列がログに表示されます。

    ![tutorial_view](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_view.png)

    上のプログラム例のように、
    テキストの表示はadd()に文字列や数値などを渡すことでできます。
    ボタンにはFuncと同様関数や関数オブジェクト(ラムダ式など)を登録できます。
    またtextInputなどを使って入力欄を表示させることもできます。
    詳細は [5-4. View](./54_view.md) を参照

</div>

## おわりに

以上で 1-1. Tutorial (Visualizing) は終わりです。
次ページ ([1-2. Tutorial (Communication)](12_tutorial_comm.md)) にはプロセス間通信に重点をおいたチュートリアルがあります。

ここで紹介していない機能として
* [6-1. Canvas2D](61_canvas2d.md)
* [6-2. Image](62_image.md)
* [6-3. Canvas3D](63_canvas3d.md)
* [6-4. RobotModel](64_robot_model.md)

もWebUIからアクセスすることのできる機能としてあるので、見てみてください。

また、チュートリアルでは紹介していないコマンドラインツールとして
* [7-1. webcface-launcher](71_launcher.md)
* [7-4. webcface-ls](74_ls.md)
* [7-5. webcface-tui](75_tui.md)

もあります。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| | [1-2. Tutorial (Communication)](12_tutorial_comm.md) |

</div>
