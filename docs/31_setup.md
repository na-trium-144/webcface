# 3-1. Setup WebCFace Library

\tableofcontents

以降、このドキュメントではクライアントの仕様変更された機能は  
C,C++のwebcface: <span class="since-c"></span>  
webcface-js: <span class="since-js"></span>  
webcface-python: <span class="since-py"></span>  
の画像で示します。

## C/C++

C/C++のライブラリはREADMEにしたがってwebcfaceをインストールすれば使うことができます。
MesonまたはCMakeを使用する場合は、subproject/subdirectoryとしてWebCFaceを追加してソースからビルドすることもできます。

<div class="tabbed">

- <b class="tab-title">Meson</b>
    \since <span class="since-c">2.0</span>

    インストールしたWebCFaceを使用する場合の例
    ```meson
    webcface_dep = dependency('webcface', version: '>=2.0.0')
    ```
    * webcfaceはpkg-configのファイル(lib/pkgconfig/webcface.pc)とcmakeのファイル(lib/cmake/webcfaceConfig.cmake)の両方をインストールし、どちらもMesonから読み込むことができます。
    * pkg-configで読み込む場合は環境変数`PKG_CONFIG_PATH`(またはmesonの引数`-Dpkg_config_path`)に (webcfaceのパス)/lib/pkgconfig を追加する必要があります。
    * cmakeで読み込む場合は環境変数`PATH`に (webcfaceのパス)/bin を追加するか、
      mesonの引数`-Dcmake_prefix_path`にwebcfaceのパスを追加する必要があります。
    * パッケージ名はどちらも小文字で`webcface`です。

    subprojectにする場合は以下のようなwrapを書き、
    ```
    [wrap-git]
    url = https://github.com/na-trium-144/webcface.git
    revision = v2.2.1
    depth = 1
    [provide]
    dependency_names = webcface
    ```
    インストールしたWebCFaceを使う場合と同様に`dependency()`で使用することができます。
    またはwebcfaceのgitリポジトリ(https://github.com/na-trium-144/webcface.git )を直接subproject/ディレクトリに置いても使えると思います。

    \note
    * C++でWebCFaceを使用するにはC++17が必要です。
    mesonの引数で `-Dcpp_std=c++17` のように指定するか、
    `project()` に `default_options: ['cpp_std=c++17'],` のように記述するとよいです。
    * staticビルドにしたり、webcface-serverのビルドの有無を切り替えたりといったオプションについては [3-2. Building from source](./32_building.md) を参照

- <b class="tab-title">CMake</b>
    インストールしたWebCFaceを使用する場合の例
    ```cmake
    find_package(webcface 2.0 CONFIG REQUIRED)

    target_link_libraries(your-target PRIVATE webcface::webcface) # C++の場合
    target_link_libraries(your-target PRIVATE webcface::wcf) # Cの場合(ver1.5.1〜)
    ```
    * find_packageでwebcfaceが見つからない場合は環境変数`PATH`に (webcfaceのパス)/bin を追加するか、
    webcfaceのインストール場所を`CMAKE_PREFIX_PATH`か`webcface_ROOT`, `webcface_DIR`などに設定してください。
    * バージョン指定は同じメジャーバージョンで指定したマイナーバージョン以上のものが選ばれます
    (例えば2.0と書いて2.1が選ばれることはあるが、1.0と書いて2.0や2.1が選ばれることはない)

    \note
    webcface::webcface ターゲットには cpp_std_17 が、 webcface::wcf ターゲットには c_std_99 の指定がそれぞれ含まれています。

    FetchContentを使う場合
    ```cmake
    FetchContent_Declare(webcface
      GIT_REPOSITORY https://github.com/na-trium-144/webcface.git
      GIT_TAG        v2.2.1
    )
    FetchContent_MakeAvailable(webcface)
    ```
    またはwebcfaceのgitリポジトリ(https://github.com/na-trium-144/webcface.git )を直接どこかのディレクトリに置いて`add_subdirectory()`でも使えると思います。
    
    \warning
    <span class="since-c">2.0</span>
    WebCFaceのリポジトリにはCMakeLists.txtがありますが、中身はcustom_targetでmesonを呼び出す仕様になっています。
    そのためFetchContentやsubdirectoryとしてWebCFaceをソースからビルドする場合はMeson(ver1.3以上)のインストールが必要になります。
    * Linux: `sudo apt install meson`(ubuntu24.04以降) または `pip install meson`
    * MacOS: `brew install meson` または `pip install meson`
    * Windows: https://github.com/mesonbuild/meson/releases からmsi形式でダウンロード、インストール

- <b class="tab-title">pkg-config</b>
    環境変数`PKG_CONFIG_PATH`に (webcfaceのパス)/lib/pkgconfig を追加し、
    * コンパイル時の引数に `$(pkg-config --cflags webcface)`
    * リンク時に `$(pkg-config --libs webcface)`
    
    を渡せばよいです

- <b class="tab-title">手動リンク</b>
    * インクルードディレクトリ(-I など): `(webcfaceのパス)\include`
    * ライブラリディレクトリ(-L など): `(webcfaceのパス)\lib`
    * Linux, MacOS
        * `-lwebcface` でリンクできるはずです。
    * Windows (MSVC)
        * リンクするライブラリは、 webcface.lib (Release) / webcfaced.lib (Debug)
        * また、`(webcfaceのパス)\bin` を環境変数のPathに追加するか、
        その中にある webcface-20.dll / webcfaced-20.dll ファイルを実行ファイルのディレクトリにコピーして読み込ませてください

</div>

### API

C++のソースコードでは`<webcface/webcface.h>`をincludeするとwebcfaceのすべての機能が使用できます。

<span class="since-c">1.10</span>
`<webcface/client.h>`, `<webcface/value.h>`など必要なヘッダファイルだけincludeして使うこともでき、コンパイル時間を短縮できます。

<span class="since-c">1.5</span>
C++ではなくCからアクセスできるAPIとして、wcf〜 で始まる名前の関数やstructが用意されています。
(C++ライブラリのうちの一部の機能しかまだ実装していませんが)

~~&lt;webcface/c_wcf.h&gt; をincludeすることで使えます。~~  
<span class="since-c">1.7</span> &lt;webcface/wcf.h&gt; をincludeすることで使えます。(c_wcf.hも一応使えます)  
ほとんどの関数は戻り値が <del>int 型</del> <span class="since-c">2.0</span> enum wcfStatus 型で、
成功した場合 0 (= <del>WCF_OK</del> <span class="since-c">2.0</span> WCF_OK)、例外が発生した場合正の値を返します。

## Python

Pythonで利用したい場合は `pip install webcface` でライブラリをインストールしてください。
Python3.8以上で動作します。

importは
```py
import webcface
```
で `webcface.Client` のように使うか、
```py
from webcface import Client
```
のように使います。

使い方はC++のライブラリとだいたい同じです。
次ページ以降のドキュメントとともに [webcface-python APIリファレンス](https://na-trium-144.github.io/webcface-python/) も参照してください

## JavaScript/TypeScript

`npm install webcface` でライブラリをインストールできます。
Node.jsでもブラウザー(webpackなど)でも動きます。

Node.jsの場合、ESMで
```js
import { Client } from "webcface";
```
のようにして使うことができます。
CommonJSで`requires("webcface")`のように使うこともできると思います。

<span class="since-js">1.7</span>
ブラウザー上で使う場合はCDNも利用できます。
```html
<script src="https://cdn.jsdelivr.net/npm/webcface@1.7.0/dist/webcface.bundle.js"></script>
```
で読み込むとグローバルに `webcface.Client` などが使用できるようになります。

使い方はC++のライブラリとだいたい同じです。
次ページ以降のドキュメントとともに [webcface-js APIリファレンス](https://na-trium-144.github.io/webcface-js/) も参照してください。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [2-2. WebUI](22_webui.md) | [3-2. Building from Source](32_building.md) |

</div>
