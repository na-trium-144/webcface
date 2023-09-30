# View

API Reference → WebCFace::View

Viewデータを送受信します。

Viewは文字列やボタンなどの要素(ViewComponent)を並べて、オリジナルのUIを表示できる機能です。
(使用例はReadmeを参照)

Member::view() でViewクラスのオブジェクトが得られます
```cpp
WebCFace::View view_hoge = wcli.member("a").view("hoge");
```

Member::views() でそのMemberが送信しているviewのリストが得られます
```cpp
for(const WebCFace::View &v: wcli.member("a").views()){
	// ...
}
```

Member::onViewEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onViewEntry().appendListener([](WebCFace::View v){ /* ... */ });
```

## 送信

自分自身の名前のMemberからViewオブジェクトを作り、
View::add() などで要素を追加し、
Viewオブジェクトのデストラクタが呼ばれた後のClient::sync()で送信されます
```cpp
{
	WebCFace::View v = wcli.view("hoge");
	v.add(...);
	v.add(...);
}
// ...
wcli.sync();
```

Viewはstd::ostreamを継承しており、 add() の代わりに v << 表示する値; というようにもできます

Viewオブジェクトを毎回生成・破棄する代わりに、 View::init() でViewを初期化、 View::sync() で反映することもできます。

## ViewComponent
`using namespace WebCFace::ViewComponents;`をすると便利かもしれません

### text
文字列です。そのまま表示します。
`WebCFace::ViewComponents::text(文字列)` の他、ostreamでフォーマット可能なデータはそのまま渡して文字列化できます

### newLine
改行します。
`WebCFace::ViewComponents::newLine()` の他、`std::endl`や`"\n"`でも改行できます

### button
ボタンを表示します。

第2引数に関数を登録済みの[Funcオブジェクト](./30_func.md)、または関数を設定することでクリック時の動作を設定できます

(AnonymousFuncオブジェクトを渡せるようにする予定だが未実装)
```cpp
WebCFace::ViewComponents::button("表示する文字列", wcli.func("func name"));
// WebCFace::ViewComponents::button("表示する文字列", wcli.func([](){ /* ... */ })); 未実装
WebCFace::ViewComponents::button("表示する文字列", [](){ /* ... */ });
```

### プロパティ
文字色や背景色など各Componentに共通のプロパティがあります。( WebCFace::ViewComponent を参照)


## 受信

ValueやTextと同様、 View::tryGet() で受信した値を取得できます。
(これを使うのはViewを表示するアプリを作る場合などですかね)

初回の呼び出しではまだ受信していないためstd::nulloptを返します。
その後sync()するとサーバーにリクエストが送信され、それ以降値が得られるようになります。

View::get() はstd::nulloptの代わりにデフォルト値を返す点以外は同じです。

## 受信イベント

View::appendListener() などで受信したデータが変化したときにコールバックを呼び出すことができます

データが変化したどうかに関わらず sync() されたときにコールバックを呼び出したい場合は Member::onSync() が使えます

また、 View::time() でその値が送信されたとき(sync()されたとき)の時刻が得られます。


