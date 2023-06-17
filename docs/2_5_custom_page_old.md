# カスタムページ(旧)
C++側でページのレイアウトを定義し、フロントエンドにカスタムのページを表示することができます。
定義は初期化時の1回でよいです。

see also WebCFace::Layout

ヘッダーは`<webcface/layout.hpp>`

## レイアウト

例
```cpp
// C++
{
  using namespace WebCFace::Layout;
  // clang-format off
  WebCFace::addPageLayout("ページ名", {{ // ←2重波括弧(重要)
      {{ 1, false, "hello", }}, // ←そのまま表示される 
      {{"xの値は", "x"_value = x, }}, // ← 変数xの値を登録して表示
      Stack{{ 1, 2, 3, }}, //←縦に並ぶ
      {{
        Button("ボタン1", "function1"_callback = func1), // 押すとfunc1を実行
        Button("ボタン2", ("function2"_callback.arg("a") = func2).arg(3)), // 押すとfunc2(引数を1つ取る)に引数として「3」を渡して実行
        Button("ボタン3", "function3"_callback).MuiColor("error"), //赤色のボタン
      }},
      ...
  }});
  // clang-format on
}
```
```py
webcface.add_page_layout("ページ名", [
  [1, False, "hello", ],
  ["xの値は", webcface.RegisterValue("x", value=x), ],
  webcface.Stack([1, 2, 3]),
  [
    webcface.Button("ボタン1", webcface.RegisterCallback("function1", callback=func1)),
    webcface.Button("ボタン2", webcface.RegisterCallback("function2", arg={"a":int}, callback=func2).arg(3)),
    webcface.Button("ボタン3", webcface.RegisterCallback("function3"), mui_color="error"),
  ]
])
```
のように、`WebCFace::Layout::addPageLayout` (python →`WebCFace::pybind::Layout::addPageLayout`)を作りたいページの個数分呼び出すと、フロントエンドには指定した名前のページが出現します。

* レイアウトの指定は上のようにコンポーネントを並べて表現します。
  * フロントエンドではここで定義したとおりに順番で縦に並んで表示されます。
  * コンポーネントの中身については後述
* レイアウトの定義時には`using namespace WebCFace::Layout;`をするのがおすすめです。
  * それ以降のプログラムに影響が出ないよう、レイアウト定義専用の関数を作るか、前後を`{ }`で囲うとよいと思います。
* また、clang-formatを使うと見た目が崩れてしまう場合があるので、前後に
  * `// clang-format off`
  * `// clang-format on`
  * を書くとよいです。

## Displayable
* フロントエンドに表示可能な値を表す型を WebCFace::Registration::Displayable (python→ WebCFace::pybind::Registration::PyDisplayable ) (旧DispValue)と表記します
* int, double, stringなどの値
  * 定数でも変数でも可、値渡しになる
  * Displayableの引数を取る箇所に直接値を書けば自動でキャストされます
* 登録した変数や関数を参照する
  * 登録時と同様に RegisterValue クラス、`"変数名"_value` でアクセスできます
    * 同じ変数名を指定すればいつでも同じ変数を表します。登録時のRegisterValueオブジェクトを保存しておく必要はありません。
    * 登録と表示を同時に行うことができます(レイアウトの定義内に`"変数名"_value = 関数`などと書くことができます)
    * `RegisterValue("サーバー名:変数名")`, `"サーバー名:変数名"_value`とすると別のサーバーの値を表示することができます
  * Displayableの引数を取る箇所にRegisterValueを書くことでその値を表示します
    * 実際にはバックエンド側ではなくフロントエンド側で変数の値への置き換え処理がされます
  * Displayableを引数に取る箇所に値を返すラムダ式や関数を書くと、RegisterValueに登録されます(つまり `"変数名"_value = ` などを省略できる)
* 前者は常に値が変わりませんが、後者は表示される値を変化させることができます

## RegisterCallback
* `RegisterCallback`もレイアウトに記述することができます
    * 同じ関数名を指定すればいつでも同じ関数を表します。
    * 登録と表示を同時に行うことができます(レイアウトの定義内に`"関数名"_callback = 関数`などと書くことができます)
* ボタンなどコールバックを記述する箇所に使用します
* `RegisterCallback("サーバー名:関数名")`, `"サーバー名:関数名"_callback`で別のサーバーの関数を参照することもできます

## コンポーネント
詳しい説明は各クラスのページを見てください

### Vector
WebCFace::Layout::Vector, WebCFace::pybind::Layout::PyVector

componentを横1列に並べます。


### Stack
WebCFace::Layout::Stack, WebCFace::pybind::Layout::PyStack

componentを縦に並べます。

* `addPageLayout`するときにページレイアウト全体を囲う`{{ }}`は必ずStackになります。


### Button
WebCFace::Layout::Button, WebCFace::pybind::Layout::PyButton

関数を実行するボタン
* ボタンに表示する名前(Displayable)と実行する関数(RegisterCallback)を指定し、ボタンを表示します
* 色を変更することができます
