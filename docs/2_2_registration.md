# 関数や値の登録

see also WebCFace::Registration

## 手動で関数・変数を宣言する

c++のヘッダーは`<webcface/registration.hpp>`

送受信できる値の型は、 WebCFace::serialize(), WebCFace::deserialize() を参照

### callback (フロントエンドからバックエンドの関数を呼び出す)

* 関数をwebcfaceに登録し、フロントエンドから呼び出すことができます。
* フロントエンドの「シェル関数」画面で見ることができます。
  * 引数を入力して▶をクリックかEnterで実行されます。

例
```cpp
using namespace WebCFace::Literals; // _callback を使うのに必要
"関数名"_callback = func; //引数なし
"関数名"_callback.arg("引数名1", "引数名2") = func; //引数あり
```
```py
webcface.RegisterCallback("関数名", callback=func) #引数なし
webcface.RegisterCallback("関数名", arg={"引数名1": int, "引数名2": int}, callback=func) #引数あり
```

* 上の例で示した代入演算子(c++のみ)、キーワード引数(pythonのみ)の他に、メソッドチェーンや << 演算子など何通りかの書き方が用意されています
* c++では`RegisterCallback("関数名")`の短縮形として`"関数名"_callback`があります

see also `WebCFace::Registration::RegisterCallback`(for C++), `WebCFace::pybind::Registration::PyRegisterCallback`(for python)

### (フロントエンドからバックエンドの変数の値を書き換える)
* 現在フロントエンド側でこれに対応する機能が実装されていないので使用できません

### value (バックエンドからフロントエンドに値を送る)
* フロントエンドの「グラフ表示」画面で見ることができます。
  * 変数名のチェックボックスをクリックすると選択した変数がグラフで表示されます
* 変数名を半角ピリオドで区切ると、フロントエンドではフォルダアイコンでまとめて表示されます
  * 例えば「test.a」「test.b」の2つの名前で登録すると、「test」というフォルダの下に「a」「b」の変数があるような扱いになります

```cpp
using namespace WebCFace::Literals; // _value を使うのに必要
"変数名"_value = func; //関数
"変数名"_value = var; //変数の値 (値が変わるたびに再度登録する)
"変数名"_value = *var; //変数のポインタ (値が変わっても登録は1度でよい)
```
```py
webcface.RegisterValue("変数名", ret=int, value=func) #関数 戻り値型=int
webcface.RegisterValue("変数名", ret=int, value=var) #変数や値 (型=int は省略可)
```
* c++では`RegisterValue("関数名")`の短縮形として`"関数名"_value`があります

see also `WebCFace::Registration::RegisterValue`(for C++), `WebCFace::pybind::Registration::PyRegisterValue`(for python)

## 旧登録関数

ver0.3まで使われていた関数:

* WebCFace::addFunctionToRobot
* WebCFace::addSharedVarToRobot
* WebCFace::addFunctionFromRobot
* WebCFace::addSharedVarFromRobot
* WebCFace::addValueFromRobot

## (c++)指定したファイル内の関数を一括で登録

CMakeLists.txtで以下のように登録したい関数の名前空間、ソースファイルを指定します
```cmake
webcface_generate(TARGET target名 NAMESPACE 名前空間 SOURCES cppファイル名...)
```
target名は任意です。 例:
```cmake
webcface_generate(
    TARGET example_shell
    NAMESPACE Shell
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)
```

その後、指定したtarget名をtarget_link_librariesに追加してください


さらに初期化処理で`<webcface/generated.hpp>`の
WebCFace::addGeneratedFunctions();
をします

これをすると、WEB_SOURCESに指定したcppファイル内の関数が自動で登録されます
  * ただし非対応の型を引数や戻り値にもつ関数は無視される
  * 戻り値があり、かつ引数がない関数はロボット→コントローラに値を送る関数とみなし、それ以外の関数はコントローラから実行する関数とみなされる

libclang.so not found のようなエラーになる場合は、`pip install libclang=14.0.6`をしてください(最新のlibclangだとなんかエラーになるので14にしておく)

### 旧仕様の場合
旧仕様のCMakeListsでセットアップした場合は以下

CMakeLists.txtで`add_subdirectory`の前にWEB_SOURCES変数にshellのソースを指定します
```cmake
file(GLOB WEB_SOURCES ${CMAKE_CURRENT_LIST_DIR}/shell/*.cpp)
```

デフォルトでは`Shell`名前空間の関数を読み込みますが、他の名前空間を使いたい場合はWEB_NAMESPACE変数を指定すると登録したい関数のnamespaceを変更できます
```cmake
set(WEB_NAMESPACE Shell::Hoge) # Shell::Hoge の中だけを読み込む
```

`target_link_libraries`に`webcface::generate`を追加します

C++での初期化は新仕様と同様です

## 外部ライブラリの使用

以下に示すライブラリと連携した機能がwebcface/external以下のヘッダーに入っていますが、WebCFace側ではこれらのライブラリにはリンクしません

### Protocol Buffer

`<webcface/external/protobuf.hpp>`の
WebCFace::addProtoBufValueFromRobot
を毎周期やると、messageに含まれるフィールドすべての値が再帰的にフロントエンドに送られます

### pybind11

C++でpybind11を使ってモジュールをビルドしている場合、そのモジュールにwebcfaceのAPIを追加するにはPYBIND11_MODULE内で以下のように記述
```cpp
#include <webcface/external/pybind.hpp>

PYBIND11_MODULE(モジュール名, m)
{
  // ...
  
  WebCFace::pybindModuleDef(m);
}
```
ビルドしたモジュールにpython版webcfaceのメンバ関数が追加され、使うことができます。
