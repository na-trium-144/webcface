# Message

送受信されるメッセージの仕様について

MessagePackで送受信されます。
両方向の通信ともにn個のデータを長さ2nの配列で表現します。

```js
[ kind1, data1, kind2, data2, ... ]
```

see also message.h

## Sync
### sync init (kind = 80)
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
* クライアントがsync initを送信するまで、サーバーは何も送信しません
* Mが空でない場合のみ、サーバーがm,aを設定し各クライアントに送信し、クライアントは新しいMemberのidを知ることができます
	* member idはクライアントごとにサーバーが一意に振る1以上の整数です

### svr version (kind = 88)
```js
data = {
	n: string, // server name
	v: string, // server version
}
```
* サーバーからクライアントに1回送られます

### ping (kind = 89)
```js
data = {}
```
* サーバーからクライアントに数秒に1回送られます。
* クライアントは即座にpingを送り返してください
	* 送り返さなくても通信が切断されるなどの副作用はないです

### ping status (kind = 90)
```js
data = {
	s: {member_id: status, member_id: status, ...},
}
```
* サーバーからクライアントに各クライアントのping往復時間(ms)をまとめて送ります
	* member_idとstatusの型はnumberです

### ping status req (kind = 91)
```js
data = {}
```
* クライアントからサーバーにこれを送ると、以降数秒おきにping statusが送られてくるようになります

### sync (kind = 87)
```js
data = {
	m: number; // member id
	t: number; // time
}
```
* クライアントからサーバーに、sync()時に他のデータを送信する前にtのみをセットして1度送ります
* サーバーはmをセットし全クライアントに送り返します
* timeは1970/1/1 0:00(utc) からの経過ミリ秒数で表します
	* これ以外にも時刻を送受信するときは同様です

## Func
### func info (kind = 84)
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
* クライアント→サーバーに送ると、サーバー→全クライアントに通知します
	* mはサーバーがセットします
* func infoを送らなくても関数の呼び出しは可能なので、非表示にしたい関数はfunc infoを送らなければよいです

### call (kind = 81)
```js
data = {
	i: number; // caller id
	c: number; // member id of caller
	r: number; // member id of target
	f: string; // function name
	a: any[]; // arguments
}
```
* 関数を呼び出すときにクライアント(caller)が送信すると、そのままクライアント(target)に送られます
	* cはサーバーがセットします
* iは実行結果が返ってくるときに判別できるよう、呼び出し側クライアントが任意に決めて良い数値(0以上の整数)です
* 関数の呼び出しはsyncと無関係なタイミングで構いません
* また、無名のクライアントからも送信することができます

### call response (kind = 82)
```js
data = {
	i: number; // caller id
	c: number; // member id of caller
	s: boolean; // if function is started
}
```
* targetはiとcにCallと同じものを、関数の実行を開始した時sにtrueを、関数が存在しない場合にはsにfalseをセットし、サーバーに送るとクライアント(caller)に届きます
* targetがそもそも存在しない場合はサーバーがsにfalseをセットしてcallerに送ります

### call result (kind = 83)
```js
data = {
	i: number; // caller id
	c: number; // member id of caller
	e: boolean; // error or not
	r: any; // result
}
```
* targetが関数の実行結果をeとrにセットしてサーバーに送るとcallerに届きます
	* エラーの場合eをtrueにしてエラーの内容をrにセットします
* callerはCallResponseでs=trueだった場合これが届くまで待機できます
	* s=falseだった場合CallResultは届きません

## Value
### value (kind = 0)
```js
data = {
	f: string, // name
	d: number[], // data
}
```
* クライアント→サーバーに更新されたデータを送ります

### value entry (kind = 20)
```js
data = {
	m: number, // member id
	f: string, // name
}
```
* 新しいValueが送られてきた時、サーバーは全クライアントにValueEntryを送ります

### value req (kind = 40)
```js
data = {
	m: number, // member id
	f: string, // name
	i: number, // request id
}
```
* valueを受信したいときにこれを送ると以降value resがサーバーから送られてくるようになります
* iはクライアントが任意に決めて良い1以上の整数です
	* Text, Viewなど他の種類のReqとは数値がかぶっていても問題ありません

### value res (kind = 60)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: number[],
}
```
* requestに対応するvalueを送り返します

## Text
### text (kind = 1)
```js
data = {
	f: string, // name
	d: string, // data
}
```
* value と同様

### text entry (kind = 21)
* value entryと同様

### text req (kind = 41)
* value req と同様

### text res (kind = 61)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: string,
}
```
* value resと同様

## View
### view (kind = 3)
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
* データは前回のsyncから変更された要素のみを送ってください
* lは変更されていない分も含めたviewの全要素数です
* dのindexはstring型で、要素のindexを10進数で文字列にしたものです
	* 例えば `[text("aaa"), text("bbb"), text("ccc")]` が `[text("aaa"), text("ccc"), text("bbb")]` に変更された場合のメッセージは `{"d": {"1": {"t": 0, "x": "ccc"}, "2": {"t": 0, "x": "bbb"}}}`
* ![ver1.1から](https://img.shields.io/badge/ver1.1~-00599c?logo=C%2B%2B) 
dのindexをnumber型からstring型に変更

### view entry (kind = 23)
* value entryと同様

### view req (kind = 43)
* value req と同様

### view res (kind = 63)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...},
	l: number,
}
```
* value resと同様

## Log
### log (kind = 86)
```js
data = {
	m: number, // member id
	l: {
		v: number, // level 0〜5
		t: number, // time
		m: string, // message
	}[],
}
```
* クライアント→サーバーに新しく追加されたログ差分のみ送ります
* リクエストがあればサーバー→各クライアントにそのまま送り返します
* リクエスト直後、サーバーが保持しているログの全履歴を1つのlogメッセージにまとめてクライアントに送ります

### log req (kind = 87)
```js
data = {
	M: string // member name
}
```
* logの受信をリクエストします

