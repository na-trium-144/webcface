# サーバー仕様

## httpサーバー

* `/`にアクセスすると`startServer()`で指定したディレクトリのIndex.htmlが表示される
* `/_hostinfo`にアクセスするとホスト名とipアドレスがjsonで返ってくる
  * 今はもう使ってない気がする?

## WebSocketサーバー

* `/` または `/ws` にアクセスする
* `{"msgname": "メッセージ種類", "msg": 内容}`のjson形式でデータを送受信する

### setting

* 接続時と、関数などが追加されたときにロボットから送られる
* 変数1つだけ登録した場合、メンバーが1つ(メンバー名が変数名と同じ)になる

```json
{
  "functions":[
    {"name":"関数名", "args":[
      {"name":"引数名", "type":"型"},
    ]},
  ],
  "to_robot":[
    {"name":"変数名", "value":[
      {"name":"メンバー名", "type":"型"},
    ]},
  ],
  "from_robot":[
    {"name":"変数名", "type":"型"},
  ],
  "images":[
    {"name":"名前"},
  ],
  "gamepad_button":["ボタン名",],
  "gamepad_axis":["ボタン名",],
  "related_servers":[
    {"addr":"アドレス","port":0},
  ],
  "server_name":"サーバー名",
  "version":"webcfaceバージョン",
}
```

### function

* 送ると関数が実行される
* callback_idはエラーメッセージを返すのに使う
```json
{"name":"関数名", "args":{"引数名":値, ...}, "callback_id":0}
```

### to_robot

未使用

### from_robot

* 変数の値がまとめてロボットから送られる
* 変数を複数まとめて登録している場合も別々の変数として送られる
  * 例: `addSharedVarFromRobot("a", {"x", "y"}, x, y)` → `a.x`, `a.y`という変数名になる
```json
{
  "変数名":値,
  "変数名":値,
  "timestamp":時刻,
}
```

### gamepad

```json
{
  "connected": true,
  "buttons": [true, false, ],
  "axes": [1, 0, ],
}
```

### log

* cout, cerrの内容が送られる
* 接続時にまとめてすべての内容を転送し、その後は内容が追加されるたびに追加分が送られてくる
```json
[
  {
    "timestamp":時刻,
    "text":"ログ",
    "level":0,
  },
]
```

### images
```json
{
  "名前":"data:...",
  "名前":"data:...",
}
```

### layout
```json
[
  {
    "name":"名前",
    "layout":{
      "0": {"type": "Hoge", ...},
      "1": {...},
    },
    "length": 2,
  },
]
```

### layer
```json
[
  {
    "name":"名前",
    "layer": [
      {"type": "Hoge", ...},
    ],
  },
]
```

### error

* to_robotやfunctionでエラーが出た場合送られる
```json
{
  "callback_id": 0,
  "message": "",
}
```

### dialog
```json
"Layout名"
```

### audio
```json
"data:..."
```

### ping
```json
```
