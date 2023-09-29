# Message

送受信されるメッセージの仕様について

MessagePackで送受信されます。
両方向の通信ともにn個のデータを長さ2nの配列で表現します。

```js
[ kind1, data1, kind2, data2, ... ]
```

see also message.h

## data

### name (kind = 150)
```js
data = {
	n: "client_name",
}
```

### value (kind = 0)
クライアント→サーバーへのデータの送信に使用。kind=0〜49についてはこれと同様
```js
data = {
	n: "value_name",
	d: 123,
}
```

### text (kind = 1)
```js
data = {
	n: "value_name",
	d: "aaaaa",
}
```

### recv (kind = 50 + n)
サーバー→クライアントへのデータの送信に使用。nはデータの種類
```js
data = {
	f: "client_name",
	n: "value_name",
	d: 123,
}
```

### subscribe (kind = 100 + n)
クライアント→サーバー
これを送信すると、(その名前の値があれば)以降サーバーからその値が更新されるたびに送られてくるようになる(上述のrecv)。
これ(subscribe)を送らなければrecvが送られてくることはない。
```js
data = {
	f: "client_name",
	n: "value_name",
}
```

### call (kind = 151)
クライアント→サーバー→クライアント
```js
data = {
	i: 0, // callするたびに違う値を送らなければならない
	c: "client_name", // 呼び出し元
	r: "client_name", // 呼び出し対象
	n: "func_name",
	a: ["1", ...], // string[]で送らなければならない 関数に渡すときに適切にキャスト
}
```

### call_response (kind = 152)
クライアント→サーバー→クライアント
```js
data = {
	i: 0, // 対応するcallのid
	c: "client_name", // 呼び出し元
	f: true, // 関数が存在するか
	e: false, // エラーが発生したか
	r: "aaa", // 戻り値 or エラーメッセージ (stringのみ)
}
```
