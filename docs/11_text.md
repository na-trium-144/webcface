# Text

API Reference → webcface::Text

文字列(std::string)データを送受信します。
Textクラスの使い方は[Value](./10_value.md)とほぼ同じです。

Member::text() でTextクラスのオブジェクトが得られます
```cpp
webcface::Text text_hoge = wcli.member("a").text("hoge");
```

Member::texts() でそのMemberが送信しているtextのリストが得られます
```cpp
for(const webcface::Text &v: wcli.member("a").texts()){
	// ...
}
```

Member::onTextEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onTextEntry().appendListener([](webcface::Text v){ /* ... */ });
```

## 送信

自分自身の名前のMemberからTextオブジェクトを作り、 Text::set() でデータを代入し、Client::sync()することで送信されます
```cpp
wcli.text("hoge").set("hello");
```

webcface::Text::Dict オブジェクトを使うと複数の値をまとめて送ることができます。
```cpp
struct A {
	std::string x, y;
	operator webcface::Text::Dict() const {
		return {
			{"x", x},
			{"y", y},
			// Value::Dictと同様、入れ子にもできます
		}
	}
};

A a_instance;
wcli.text("a").set(a_instance); // Dictにキャストされる
```

 (C++のみ) set() の代わりに代入演算子(Text::operator=)でも同様のことができます。

## 受信

Text::tryGet(), Text::tryGetRecurse() で値のリクエストをするとともに受信した値を取得できます。

初回の呼び出しではまだ受信していないためstd::nulloptを返します。
(pythonでは None, javascriptでは null)

その後Client::sync()したときに実際にリクエストが送信され、それ以降は値が得られるようになります。
```cpp
while(true) {
	std::optional<std::string> val = wcli.member("a").text("hoge").tryGet();
	if(val) {
		std::cout << "hoge = " << *val << std::endl;
	}
	wcli.sync();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

Text::get(), Text::getRecurse() はstd::nulloptの代わりにデフォルト値を返す点以外は同じです。

また、std::string, Value::Dict などの型にキャストすることでも同様に値が得られます。

## 受信イベント

Text::appendListener() で受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定するとその値はリクエストされます。
```cpp
wcli.member("a").text("hoge").appendListener([](Text v){ /* ... */ });
```
pythonでは Text.signal プロパティがこのイベントのsignalを返します。

データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます
```cpp
wcli.member("a").onSync().appendListener([](Member m){ /* ... */ });
```

[Value](./10_value.md) ←前 | 次→ [View](./13_view.md)
