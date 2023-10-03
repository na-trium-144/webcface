# Message

送受信されるメッセージの仕様について

MessagePackで送受信されます。
両方向の通信ともにn個のデータを長さ2nの配列で表現します。

```js
[ kind1, data1, kind2, data2, ... ]
```

see also message.h

## data

詳細な説明は WebCFace::Message:: 内の各メッセージのクラスの説明として書かれているのでそちらを参照してください

### sync init (WebCFace::Message::SyncInit, kind = 80)
```js
data = {
	M: string, // member name
	m: number, // member id
	l: string, // library name
	v: string, // library number
	a: string, // ip address
}
```
* クライアント初期化時にM,l,vのみ設定してサーバーに送信
* Mが空でない場合のみ、サーバーがm,aを設定し各クライアントに送信し、クライアントは新しいMemberのidを知ることができます

### svr version (WebCFace::Message::SvrVersion, kind = 88)
```js
data = {
	n: string, // server name
	v: string, // server version
}
```
* サーバーからクライアントに1回送られます

### ping (WebCFace::Message::Ping, kind = 89)
```js
data = {}
```
* サーバーからクライアントに数秒に1回送られます。
* クライアントは即座にpingを送り返してください
	* 送り返さなくても通信が切断されるなどの副作用はないです

### ping status (WebCFace::Message::PingStatus, kind = 90)
```js
data = {
	s: {member_id: status, member_id: status, ...},
}
```
* サーバーからクライアントに各クライアントのping往復時間(ms)をまとめて送ります

### ping status req (WebCFace::Message::PingStatusReq, kind = 91)
```js
data = {}
```
* クライアントからサーバーにこれを送ると、以降数秒おきにping statusが送られてくるようになります

### sync (WebCFace::Message::Sync, kind = 87)
```js
data = {
	m: number; // member id
	t: number; // time
}
```
* クライアントからサーバーに、sync()時に他のデータを送信する前に1度送ります

### func info (WebCFace::Message::FuncInfo, kind = 84)
```js
data = {
	m: number, // member id
	f: string, // name
	r: number, // return type
	a: {
		n: string,
		t: number,
		i: any | null,
		m: any | null,
		x: any | null,
		o: any[],
	}[],
}
```
* クライアント→サーバーに送り、サーバー→全クライアントに通知します

### call (WebCFace::Message::Call, kind = 81)
```js
data = {
	i: number; // caller id
	c: number; // member id of caller
	r: number; // member id of target
	f: string; // function name
	a: any[]; // arguments
}
```
* 関数を呼び出すときにクライアント(caller)→サーバー→クライアント(target)に送られます
* caller idは実行結果が返ってくるときに判別できるよう、呼び出し側クライアントが任意に決めて良い数値です
* 関数の呼び出しはsyncと無関係なタイミングで構いません
* また、無名のクライアントからも送信することができます
### call response (WebCFace::Message::CallResponse, kind = 82)
```js
data = {
	i: number; // caller id
	c: number; // member id of caller
	s: boolean; // if function is started
}
```
* 関数の実行を開始した時sにtrueをセットしクライアント(target)→サーバー→クライアント(caller)に送り返します
* 関数が存在しない場合sにfalseをセットし送り返します

### call result (WebCFace::Message::CallResult, kind = 83)
```js
data = {
	i: number; // caller id
	c: number; // member id of caller
	e: boolean; // error or not
	r: any; // result
}
```
* 関数の実行結果を返します
* エラーの場合エラーの内容をrにセットします

### value (WebCFace::Message::Value, kind = 0)
```js
data = {
	f: string, // name
	d: number[], // data
}
```
* クライアント→サーバーに更新されたデータを送ります


### value (WebCFace::Message::Value, kind = 0)
```js
data = {
	f: string, // name
	d: number[], // data
}
```
* クライアント→サーバーに更新されたデータを送ります

### value entry (WebCFace::Message::Entry<Value>, kind = 20)
```js
data = {
	m: number, // member id
	f: string, // name
}
```
* 新しいvalueが追加されたときにサーバー→全クライアントに通知します


### value req (WebCFace::Message::Req<Value>, kind = 40)
```js
data = {
	m: number, // member id
	f: string, // name
	i: number, // request id
}
```
* valueを受信したいときにこれを送ると以降value res がサーバーから送られてくるようになります

### value res (WebCFace::Message::Res<Value>, kind = 60)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: number[],
}
```
* requestに対応するvalueを送り返します

### value (WebCFace::Message::Value, kind = 0)
```js
data = {
	f: string, // name
	d: number[], // data
}
```
* クライアント→サーバーに更新されたデータを送ります


### text (WebCFace::Message::Text, kind = 1)
```js
data = {
	f: string, // name
	d: string, // data
}
```
* value と同様

### text entry (WebCFace::Message::Entry<Text>, kind = 21)
* value entryと同様

### text req (WebCFace::Message::Req<Text>, kind = 41)
* value req と同様

### text res (WebCFace::Message::Res<Text>, kind = 61)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: string,
}
```
* value resと同様

### view (WebCFace::Message::View, kind = 3)
```js
data = {
	f: string, // name
	d: {
		index: {
			t: number,
			x: string,
			L: string | null,
			l: string | null,
			c: number,
			b: number,
		},
		index: {},
		...
	},
	l: number, // data length
}
```
* value と同様
* データは前回のsyncから変更された要素のみを送る

### view entry (WebCFace::Message::Entry<View>, kind = 23)
* value entryと同様

### view req (WebCFace::Message::Req<View>, kind = 43)
* value req と同様

### view res (WebCFace::Message::Res<View>, kind = 63)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...},
	l: number,
}
```
* value resと同様

### log (WebCFace::Message::Log, kind = 86)
```js
data = {
	m: number, // member id
	l: { v: number, t: number, m: string }[],
}
```
* クライアント→サーバーに新しく追加されたログのみ送ります
* リクエストがあればサーバー→各クライアントに送り返します

### log req (WebCFace::Message::LogReq, kind = 87)
```js
data = {
	M: string // member name
}
```
* logの受信をリクエストします

