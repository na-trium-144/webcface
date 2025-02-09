# 8-1. Message

\tableofcontents

送受信されるメッセージの仕様について

MessagePackで送受信されます。
両方向の通信ともに、一度にn個のデータを送信する場合それを以下のように長さ2nのArrayとしてシリアライズしたものを送信します。
kindは0以上の整数、データはMapです。

```js
[ kind1, data1, kind2, data2, ... ]
```

stringは常にutf-8エンコーディングとします。

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

### sync init end (kind = 88)

(ver1.11まで: svr version)

```js
data = {
	n: string, // server name
	v: string, // server version
	m: number, // member id
}
```
* <del>サーバーからクライアントに1回送られます</del>
* <span class="since-c">2.0</span>
クライアントからサーバーに sync init が送られたあと、
サーバーはクライアントにすべてのentryを送り、最後に sync init end を送ります
	* クライアントはwaitConnection()でこれが送られてくるまで待機します
* <span class="since-c">2.0</span> member id 追加 (sync init をしたメンバーのid)

### closed (kind = 93)
\since <span class="since-c">2.9</span>

```js
data = {
	m: number, // member id
}
```
* クライアントが切断したとき、サーバーからクライアントに送られます

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
	* syncと同じ配列に入れて送られたデータはすべて同時刻のデータとみなされます
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
	i: number, // index
}
```
* クライアント→サーバーに送ると、サーバー→全クライアントに通知します
	* mはサーバーがセットします
* func infoを送らなくても関数の呼び出しは可能なので、非表示にしたい関数はfunc infoを送らなければよいです
* <span class="since-c">2.8</span> index 追加 (関数を登録した順番)

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
	M: number, // member id
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
	d: string | number | boolean, // data
}
```
* value と同様
* <span class="since-c">1.10</span> dのデータ型をstringのみからnumberとbooleanも含むように変更

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
### view (kind = 9)

\since <span class="since-c">2.5</span>

```js
data = {
	f: string, // name
	d: {
		id: {
			t: number, // type
			x: string, // text
			L: string | null, // onClick Func.member
			l: string | null, // onClick Func.name
			c: number, // textColor
			b: number, // bgColor
			R?: string | null, // Bind Text.member
			r?: string | null, // Bind Text.name
			im?: number | null, // min
			ix?: number | null, // max
			is?: number | null, // step
			io?: string[] | number[], // option
			w?: number, // width
			h?: number, // height
		},
		id: {...},
		...
	},
	l: null | string[], // list of id
}
```
* value と同様
* 各要素にはid(string)が割り当てられており、それぞれの要素のデータ(d)とidの並び順(l)に分けて送信します
* データは前回のsyncから変更された要素のみを送ります
* idの並び順はviewの全要素のidのリスト、または前回から変更されていない場合はnullを送ります
* <span class="since-c">2.6</span><span class="since-js">1.10</span><span class="since-py">3.1</span> w (width), h (height) 追加

### view entry (kind = 29)
* value entryと同様

### view req (kind = 49)
* value req と同様

### view res (kind = 69)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...},
	l: null | string[],
}
```
* value resと同様

### view old (kind = 3)

* ver2.4まで使っていた古い仕様です
	* 2.5以降のサーバーは、古いview_reqメッセージを送ってきたクライアントに対してのみこの古い仕様でViewを返します。

```js
data = {
	f: string, // name
	d: {
		index: {
			t: number, // type
			x: string, // text
			L: string | null, // onClick Func.member
			l: string | null, // onClick Func.name
			c: number, // textColor
			b: number, // bgColor
			R?: string | null, // Bind Text.member
			r?: string | null, // Bind Text.name
			im?: number | null, // min
			ix?: number | null, // max
			is?: number | null, // step
			io?: string[] | number[], // option
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
* <span class="since-c">1.1</span> dのindexをnumber型からstring型に変更
* <span class="since-c">1.10</span> data内の bind,min,max,step,option 追加

### view old entry (kind = 23)
### view old req (kind = 43)
### view old res (kind = 63)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...},
	l: number,
}
```

## Image

\since <span class="since-c">1.3</span>

### image (kind = 5)
```js
data = {
	f: string, // name
	d: ArrayBuffer, // data (uchar[])
	w: number, // width
	h: number, // height
	l: number, // color mode
	p: number, // compression mode
}
```

### image entry (kind = 25)
* value entryと同様

### image req (kind = 45)
```js
data = {
	M: number, // member id
	f: string, // name
	i: number, // request id
	w: number | null, // width
	h: number | null, // height
	l: number | null,  // color mode
	p: number, // compression mode
	q: number, // quality
	r: number, // frame rate
}
```

### image res (kind = 65)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: ArrayBuffer, // data (uchar[])
	w: number, // width
	h: number, // height
	l: number, // color mode
	p: number, // compression mode
}
```
* value resと同様

## Canvas2D

### canvas2d (kind = 10)

\since <span class="since-c">2.5</span>

```js
data = {
	f: string, // name
	w: number, // width
	h: number, // height
	d: {
		id: {
			t: number, // type
			op: number[2], // origin pos
			or: number, // origin rot
			c: number, // stroke color
			f: number, // fill color / text color
			s: number, // stroke width / font size
			gt: number | null, // geometry type
			gp: number[], // geometry properties
			L?: string | null, // onclick Func.member
			l?: string | null, // onclick Func.name
			x?: string, // text
		},
		id: {},
		...
	},
	l: null | string[], // list of id
}
```
* viewと同様、各要素にはid(string)が割り当てられており、それぞれの要素のデータ(d)とidの並び順(l)に分けて送信します
* データは前回のsyncから変更された要素のみを送ります
* idの並び順はviewの全要素のidのリスト、または前回から変更されていない場合はnullを送ります
* geometry propertiesの要素数と内容はgeometryの種類によって異なる
* textの色はfillの色、textのサイズはstrokeWidthのデータを流用

### canvas2d entry (kind = 30)
* value entryと同様

### canvas2d req (kind = 50)
* value req と同様

### canvas2d res (kind = 70)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	w: number, // width
	h: number, // height
	d: {...},
	l: number,
}
```
* value resと同様


### canvas2d old (kind = 4)

\since <span class="since-c">1.6</span>

* ver2.4まで使っていた古い仕様です
	* 2.5以降のサーバーは、古いcanvas2d_reqメッセージを送ってきたクライアントに対してのみこの古い仕様でCanvas2Dを返します。

```js
data = {
	f: string, // name
	w: number, // width
	h: number, // height
	d: {
		index: {
			t: number, // type
			op: number[2], // origin pos
			or: number, // origin rot
			c: number, // stroke color
			f: number, // fill color / text color
			s: number, // stroke width / font size
			gt: number | null, // geometry type
			gp: number[], // geometry properties
			L?: string | null, // onclick Func.member
			l?: string | null, // onclick Func.name
			x?: string, // text
		},
		index: {},
		...
	},
	l: number, // data length
}
```
* viewと同様、前回のsyncから変更された要素のみを送る
* lは変更されていない分も含めたcanvasの全要素数
* dのindexはstring型で、要素のindexを10進数で文字列にしたもの
* geometry propertiesの要素数と内容はgeometryの種類によって異なる
* <span class="since-c">1.9</span> onClickとtext追加
* textの色はfillの色、textのサイズはstrokeWidthのデータを流用

### canvas2d old entry (kind = 24)
### canvas2d old req (kind = 44)
### canvas2d old res (kind = 64)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	w: number, // width
	h: number, // height
	d: {...},
	l: number,
}
```

## Canvas3D

### canvas3d (kind = 11)

\since <span class="since-c">2.5</span>

```js
data = {
	f: string, // name
	d: {
		id: {
			t: number, // type
			op: number[3], // origin pos
			or: number[3], // origin rot
			c: number, // color
			gt: number | null, // geometry type
			gp: number[], // geometry properties
			fm: string | null, // field member
			ff: string | null, // field name
			a: { name: angle, ... } // joint angles
		},
		id: {},
		...
	},
	l: null | string[], // list of id
}
```
* viewと同様、各要素にはid(string)が割り当てられており、それぞれの要素のデータ(d)とidの並び順(l)に分けて送信します
* データは前回のsyncから変更された要素のみを送ります
* idの並び順はviewの全要素のidのリスト、または前回から変更されていない場合はnullを送ります
* geometry propertiesの要素数と内容はgeometryの種類によって異なる
* robotmodelを参照する場合そのモデルのmemberとfield名をfmとffにセット

### canvas3d entry (kind = 31)
* value entryと同様

### canvas3d req (kind = 51)
* value req と同様

### canvas3d res (kind = 71)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...},
	l: null | string[],
}
```
* value resと同様

### canvas3d old (kind = 7)

\since <span class="since-c">1.4</span>

* ver2.4まで使っていた古い仕様です
	* 2.5以降のサーバーは、古いcanvas3d_reqメッセージを送ってきたクライアントに対してのみこの古い仕様でCanvas3Dを返します。

```js
data = {
	f: string, // name
	d: {
		index: {
			t: number, // type
			op: number[3], // origin pos
			or: number[3], // origin rot
			c: number, // color
			gt: number | null, // geometry type
			gp: number[], // geometry properties
			fm: string | null, // field member
			ff: string | null, // field name
			a: { name: angle, ... } // joint angles
		},
		index: {},
		...
	},
	l: number, // data length
}
```
* viewと同様、前回のsyncから変更された要素のみを送る
* lは変更されていない分も含めたcanvasの全要素数
* dのindexはstring型で、要素のindexを10進数で文字列にしたもの
* geometry propertiesの要素数と内容はgeometryの種類によって異なる
* robotmodelを参照する場合そのモデルのmemberとfield名をfmとffにセット

### canvas3d old entry (kind = 27)
### canvas3d old req (kind = 47)
### canvas3d old res (kind = 67)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...},
	l: number,
}
```

## RobotModel

\since <span class="since-c">1.4</span>

### robotmodel (kind = 6)
```js
data = {
	f: string, // name
	d: { // links data
		n: string, // link name
		jn: string, // joint name
		jp: number, // parent link index
		jt: number, // joint type
		js: number[3], // joint origin pos
		jr: number[3], // joint origin rot
		ja: number, // joint angle
		gt: number, // geometry type
		gp: number[], // geometry properties
		c: number, // color
	}[],
}
```

### robotmodel entry (kind = 26)
* value entryと同様

### robotmodel req (kind = 46)
* value req と同様

### robotmodel res (kind = 66)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	d: {...}[], // data
}
```

## Log
### log (kind = 8)
\since <span class="since-c">2.4</span>

```js
data = {
	f: string, // name
	l: {
		v: number, // level 0〜5
		t: number, // time
		m: string, // message
	}[],
}
```
* クライアント→サーバーに新しく追加されたログ差分のみ送ります

### log entry (kind = 28)
* value entryと同様

### log req (kind = 48)
* value reqと同様

### log res (kind = 68)
```js
data = {
	i: number, // request id
	f: string, // sub field name
	l: {...}[], // data
}
```
* 初回はすべてのログが送られますが、2回目以降は前回送信した log res の後に追加された差分のみが送られます

### log_default (kind = 85)
* ver2.3まで使っていた古いメッセージ仕様です。
	* 2.4以降のサーバーは、古いlog_reqメッセージを送ってきたクライアントに対してのみこの古い仕様でログを返します。
	* またこの古いlogメッセージは新logメッセージでnameを`"default"`にしたものと同等に扱われます

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

### log_entry_default (kind = 92)
\since <span class="since-c">2.1</span>

* ver2.1〜2.3まで使っていた古いメッセージ仕様です。
	* 2.4以降のサーバーは、nameが`"default"`のLogデータに関してのみ、新旧両方のクライアントに対応できるようこのentryと新しいlog entryの両方を送信します

```js
data = {
	m: number // member id
}
```
* 1行以上のログデータがある場合、サーバー→各クライアントに知らせます

### log_req_default (kind = 86)
* ver2.3まで使っていた古いメッセージ仕様です。
	* 2.4以降のサーバーは、nameが`"default"`のLogをリクエストしたものとして処理します

```js
data = {
	M: string // member name
}
```
* logの受信をリクエストします

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [7-6. webcface-notepad](76_notepad.md) | [8-2. Client Spec](82_client_spec.md) |

</div>
