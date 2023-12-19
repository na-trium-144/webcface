# Member

API Reference → webcface::Member

WebCFaceではサーバーに接続されたそれぞれのクライアントを Member と呼びます。
(たぶんROSでいうと Node に相当します)

データを受信する時など、Memberを指すために使用するのがMemberクラスです。
Client::member() で取得できます。

```cpp
webcface::Member member_a = wcli.member("a");
```
これは`a`という名前のMember(=Clientのコンストラクタに`a`を入力したクライアント)を指します。

Memberクラスから実際にそれぞれのデータにアクセスする方法は次ページ以降で説明します。

このクライアント自身もMemberの1つです。
各種データを送信するにも(一部例外はありますが)Memberクラスを経由する必要があります。
Client::member()の引数に自身の名前を入れてもよいですが、Client自体がMemberを継承したクラスになっているので、キャストするか直接wcliに対して操作すればよいです。
```cpp
webcface::Member member_self = wcli;
```

Client::members() で現在接続されているメンバーのリストが得られます
(無名のものと、自分自身を除く)
```cpp
for(const webcface::Member &m: wcli.members()){
	// ...
}
```

## Field系クラスの扱いについて

Memberクラスおよびこれ以降説明する各種データ型のクラス (いずれも webcface::Field を継承している) について、

* それぞれコンストラクタが用意されていますが、正しくClientクラスから生成したオブジェクトでないと内部のデータにアクセスしようとするときに std::runtime_error (pythonでは RuntimeError) を投げます。
* 構築元のClientの寿命が切れた後に操作しようとすると同様にstd::runtime_errorを投げます。
* オブジェクトのコピー、ムーブは可能です。

## Event

Client::onMemberEntry() で新しいメンバーが接続されたときのイベントにコールバックを設定できます
```cpp
wcli.onMemberEntry().appendListener([](webcface::Member m){/* ... */});
```
このクライアントが接続する前から存在したメンバーについては start() 後に一度に送られるので、
コールバックの設定はstart()より前に行うと良いです。

onMemberEntry() はC++では EventTarget クラスのオブジェクトを返します。
内部ではイベントの管理に [eventpp](https://github.com/wqking/eventpp) ライブラリを使用しており、EventTargetは eventpp::EventDispatcher のラッパーとなっています。

* pythonでは client.on_member_entry プロパティが [blinker](https://pypi.org/project/blinker/) ライブラリの signal を返します。
* javascriptでは Client.onMemberEntry プロパティが返す EventTarget クラスのオブジェクトがEventEmitterのラッパーになっています。

これ以降の章でもいくつかイベントが登場しますが、いずれもこれと同様の実装、使い方になっています。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Client](01_client.md) | [Value](10_value.md) |

</div>
