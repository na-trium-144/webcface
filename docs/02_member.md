# Member

API Reference → WebCFace::Member

同じサーバーに接続されている他のクライアントを表します。

Client::member() で取得できます
```cpp
WebCFace::Member member_a = wcli.member("a");
```
これは`a`という名前のクライアントを指します

このクライアント自身を表すMemberは引数に自身の名前を入れるか、または Client 自体がMemberを継承したクラスになっているのでそこからも得られます
(ただし自分自身を表すMemberでは一部正常に機能しないプロパティがあります)
```cpp
WebCFace::Member member_self = wcli;
```

Client::members() で現在接続されているメンバーのリストが得られます
(無名のものと、自分自身を除く)
```cpp
for(const WebCFace::Member &m: wcli.members()){
	// ...
}
```

Client::onMemberEntry() で新しいメンバーが接続されたときのコールバックを設定できます
```cpp
wcli.onMemberEntry().appendListener([](WebCFace::Member m){/* ... */});
```


