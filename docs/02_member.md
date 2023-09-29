# Member

API Reference → WebCFace::Member

同じサーバーに接続されている他のクライアントを表します。

Client::member() で取得できます
```cpp
WebCFace::Member member_a = wcli.member("a");
```
これは`a`という名前のクライアントを指します

Client::members() で現在接続されているメンバーのリストが得られます
(無名のものを除く)
```cpp
for(const WebCFace::Member &m: wcli.members()){
	// ...
}
```

Client::onMemberEntry() で新しいメンバーが接続されたときのコールバックを設定できます
```cpp
wcli.onMemberEntry().appendListener([](WebCFace::Member m){/* ... */});
```


