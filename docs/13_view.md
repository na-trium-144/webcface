# View

API Reference → webcface::View, webcface::ViewComponent

Viewデータを送受信します。

Viewは文字列やボタンなどの要素(ViewComponent)を並べて、オリジナルのUIを表示できる機能です。
(使用例はTutorialを参照)

Member::view() でViewクラスのオブジェクトが得られます
```cpp
webcface::View view_hoge = wcli.member("a").view("hoge");
```

Member::views() でそのMemberが送信しているviewのリストが得られます
```cpp
for(const webcface::View &v: wcli.member("a").views()){
	// ...
}
```

Member::onViewEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onViewEntry().appendListener([](webcface::View v){ /* ... */ });
```

## 送信

自分自身の名前のMemberからViewオブジェクトを作り、
View::add() などで要素を追加し、
最後にView::sync()をしてからClient::sync()をすることで送信されます。

Viewはstd::ostreamを継承しており、 add() の代わりに v << 表示する値; というようにもできます。
ostreamに出力可能なものはそのままviewに出力できます。
ostreamと同様にフォーマットを指定したり、std::endlで改行もできます。

```cpp
webcface::View v = wcli.view("hoge");
// v.init(); // ←オブジェクトvを新規に構築せず繰り返し使いまわす場合は必要
v << ...; // v.add(...) と等価
v << ...;
v.sync(); // ここまでにvに追加したものをクライアントに反映

wcli.sync();
```
C++ではViewのデストラクタでも自動的にView.sync()が呼ばれます。  
![c++ ver1.2](https://img.shields.io/badge/1.2~-00599c?logo=C%2B%2B)
Viewオブジェクトをコピーした場合、Viewオブジェクトの内容はコピーされるのではなく共有され、そのすべてのコピーが破棄されるまでsync()は呼ばれません。
```cpp
{
	webcface::View v = wcli.view("hoge");
	v << ...;
	v << ...;
	// v.sync(); (自動で呼ばれる)
}

wcli.sync();
```


Pythonでも使い方はC++と同様です。
Pythonではwith構文を使って `with wcli.view("hoge") as v:` などとするとwithを抜けるときに自動でv.sync()がされます。
```py
v = wcli.view("hoge")
# v.init(); ←オブジェクトvを新規に構築せず繰り返し使いまわす場合は必要
v.add(...)
v.sync()
```
```py
with wcli.view("hoge") as v:
    v.add(...)
    v.add(...)
    # v.sync() (自動で呼ばれる)
```

JavaScriptの場合、add(), init(), sync()は用意されておらず、set()の引数に要素をまとめてセットして使います。
```js
wcli.view("hoge").set([
	"hello",
	123,
	viewComponents.button("aaa", () => undefined)
]);
```

## ViewComponent
Viewに追加する各種要素をViewComponentといいます。

* C++では `webcface::ViewComponents` 名前空間に定義されています。 `using namespace webcface::ViewComponents;`をすると便利かもしれません
* Pythonでは `webcface.view_somponents` モジュール内にあり、`from webcface.view_components import *` ができます
* JavaScriptでは `viewComponents` オブジェクト内にあります

### text
文字列です。そのまま表示します。
`webcface::ViewComponents::text(文字列)` の他、ostreamでフォーマット可能なデータはそのまま渡して文字列化できます。  
View::add()関数, set()関数では数値やbool値は文字列に変換されます。

以下はいずれも「hello」という文字列を表示します。
```cpp
v.add("hello");
v << "hello";
v << "he" << "llo";
using namespace webcface::ViewComponents;
v.add(text("hello"));
v << text("hello");
```

### newLine
改行します。
`webcface::ViewComponents::newLine()` の他、`std::endl`や`"\n"`でも改行できます

文字列中に`\n`があるとそこで改行されます。
以下はいずれも「hello」を2行表示します。
```cpp
v.add("hello\nhello");
v.add("hello").add("\n").add("hello");
v << "hello\nhello";
v << "hello" << std::endl << "hello";
using namespace webcface::ViewComponents;
v.add(text("hello")).add(newLine()).add(text("hello"));
v << text("hello") << newLine() << text("hello");
```

### button
ボタンを表示します。

第2引数に関数を登録済みの[Funcオブジェクト](./30_func.md)、または関数を設定することでクリック時の動作を設定できます

(AnonymousFuncオブジェクトを渡せるようにする予定だが未実装)
```cpp
using namespace webcface::ViewComponents;
v.add(button("表示する文字列", wcli.func("func name")));
// v.add(button("表示する文字列", wcli.func([](){ /* ... */ }))); 未実装
v.add(button("表示する文字列", [](){ /* ... */ }));
```

### プロパティ
文字色や背景色など各Componentに共通のプロパティがあります。( webcface::ViewComponent を参照)  
文字列の表示に対しても、明示的に`text(文字列)`という書き方にすればオプションを追加できます。

C++では
```cpp
button("文字列", func).textColor(webcface::ViewColor::white).bgColor(webcface::ViewColor::red)
```
のようにメソッドチェーンして指定できます。

Pythonでは
```python
button("文字列", func, text_color=ViewColor.WHITE, bg_color=ViewColor.RED)
```
のようにキーワード引数で設定できます。

JavaScriptでは引数にオブジェクトを渡して
```js
button("文字列", func, { textColor: viewColor.white, bgColor: viewColor.red })
```
のように指定できます。

## 受信

ValueやTextと同様、 View::tryGet() で受信したViewデータを取得できます。
(これを使うのはViewを表示するアプリを作る場合などですかね)

Viewデータは ViewComponent のリストとして得られます。

View::get() はstd::nulloptの代わりにデフォルト値を返す点以外は同じです。

## 受信イベント

View::appendListener() で受信したデータが変化したときにコールバックを呼び出すことができます。
(Pythonでは View.signal)

データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Text](11_text.md) | [Image](15_image.md) |

</div>
