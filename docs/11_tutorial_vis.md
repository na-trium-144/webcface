# 1-1. Tutorial (Visualizing)

\tableofcontents

WebCFaceの機能紹介・チュートリアルです。

このチュートリアルはWebUIでデータを可視化する・WebUIからプログラムを操作するという部分をメインにしたものです。
プロセス間通信に重点をおいたチュートリアルは [1-2. Tutorial (Communication)](./12_tutorial_comm.md)

このチュートリアルでは C++ (Meson または CMake)、または Python を使用します。

## WebCFaceのインストール

READMEにしたがって webcface, webcface-webui, webcface-tools をインストールしましょう。

C++の場合はインストールしたwebcfaceパッケージにクライアントライブラリも含まれていますが、
Pythonで利用したい場合は別途 `pip install webcface` でクライアントライブラリをインストールしてください。

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

    ターミナルを開いて
    ```sh
    webcface-server
    ```
    コマンドでサーバーが起動します。
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

    int main() {
        webcface::Client wcli("tutorial");
        wcli.waitConnection();

        std::cout << "Hello, World!" << std::endl;
    }
    ```
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

    int main() {
        webcface::Client wcli("tutorial");
        wcli.waitConnection(); // serverに接続できるまで待機

        std::cout << "Hello, World!" << std::endl;
    }
    ```
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

</div>

## Log

コンソールに出力していた `Hello, World!` を、WebCFaceにも送信してみましょう。

<div class="tabbed">

- <b class="tab-title">C++</b>

    * main.cc
    ```cpp
    #include <iostream>
    #include <webcface/client.h>

    int main() {
        webcface::Client wcli("tutorial");
        wcli.waitConnection();
        std::ostream &logger = wcli.loggerOStream();
        // loggerOStream() は std::cout と同様に文字列を出力して使うことができる

        logger << "Hello, World!" << std::endl;

        wcli.sync(); // wcli に書き込んだデータを送信する
    }
    ```

    これを実行すると、コンソール (std::coutではなくstd::cerrと同じ標準エラー出力ですが) には今まで通り `Hello, World!` と表示されます。

    それに加えて、WebUI右上のメニューから「tutorial」を開き「Logs」をクリックすると、ログを表示する画面が開きこちらからも `Hello, World!` を確認できるはずです。

    ![tutorial_log](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_log.png)

    \note
    std::wostream を使うこともできます。
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

    int main() {
        webcface::Client wcli("tutorial");
        wcli.waitConnection();
        std::ostream &logger = wcli.loggerOStream();

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

</div>

## Func

プログラムからWebUIに情報を送信するだけでなく、WebUIからプログラムを操作することも可能です。
