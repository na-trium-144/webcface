# Value

API Reference → WebCFace::Value

数値(double)データ、または数値の配列(std::vector<double>)を送受信します。

Member::value() でValueクラスのオブジェクトが得られます
```cpp
WebCFace::Value value_hoge = wcli.member("a").value("hoge");
```
これは`a`というクライアントの`hoge`という名前のデータを表します

Member::values() でそのMemberが送信しているvalueのリストが得られます
```cpp
for(const WebCFace::Value &v: wcli.member("a").values()){
	// ...
}
```

Member::onValueEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onValueEntry().appendListener([](WebCFace::Value v){ /* ... */ });
```

## 送信

自分自身の名前のMemberからValueオブジェクトを作り、 Value::set() でデータを代入し、sync()することで送信されます
```cpp
wcli.value("hoge").set(5);
wcli.value("fuga").set({1, 2, 3, 4, 5}); // std::vector<double>
```

Dict オブジェクトを使うと複数の値をまとめて送ることができます。
これは構造体などのデータを送るときに使えます
```cpp
struct A {
	double x, y;
	operator WebCFace::Value::Dict() const {
		return {
			{"x", x},
			{"y", y},
			{"vec", {1, 2, 3}}, // vectorも入れられます
			{"a", {             // 入れ子にもできます
				{"a", 1},
				{"b", 1},
			}}
		}
	}
};

A a_instance;
wcli.value("a").set(a_instance); // Dictにキャストされる

/* 結果は以下のようになる
  value("a.x") -> a_instance.x
  value("a.y") -> a_instance.y
  value("a.vec") -> {1, 2, 3}
  value("a.a.a") -> 1
  value("a.a.b") -> 1
*/
```

set() の代わりに代入演算子(Value::operator=)でも同様のことができます

また、 operator+= など、doubleやintの変数で使える各種演算子も使えます

## 受信

Value::tryGet(), Value::tryGetVec(), Value::tryGetRecurse() で受信した値を取得できます。
それぞれ 1つの値、vector、Dict を返します。

初回の呼び出しではまだ受信していないためstd::nulloptを返します。
その後sync()するとサーバーにリクエストが送信され、それ以降値が得られるようになります。
```cpp
while(true) {
	std::optional<double> val = wcli.member("a").value("hoge").tryGet();
	if(val) {
		std::cout << "hoge = " << *val << std::endl;
	}
	wcli.sync();
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
```

Value::get(), Value::getVec(), Value::getRecurse() はstd::nulloptの代わりにデフォルト値を返す点以外は同じです。

また、doubleやstd::vector<double>, Value::Dict などの型にキャストすることでも同様に値が得られます。

## 受信イベント

Value::appendListener() などで受信したデータが変化したときにコールバックを呼び出すことができます
```cpp
wcli.member("a").value("hoge").appendListener([](Value v){ /* ... */ });
```

データが変化したどうかに関わらず sync() されたときにコールバックを呼び出したい場合は Member::onSync() が使えます
```cpp
wcli.member("a").onSync().appendListener([](Member m){ /* ... */ });
```

また、 Value::time() でその値が送信されたとき(sync()されたとき)の時刻が得られます。


