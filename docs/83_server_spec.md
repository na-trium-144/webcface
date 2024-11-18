# 8-3. Server Spec

\tableofcontents

WebCFace Server の動作仕様についてです。
各メッセージのデータ型の説明 [8-1. Message](./81_message.md) も参照しながら読んでください。

## 接続

* デフォルトでは7530ポートでlistenします。
* WebSocket接続を受け付け、 sync init (80) のメッセージが送られてきたら通信開始です。
* 新しく接続してきたクライアントに対して、すでに接続中の他のメンバーに関する情報 (sync init (80), 各種データ型の entry (20〜) ) を送信します。
* そしてその新しいクライアントに member id を割り当て、 sync init end (88) を送ります。
    * id は無名のメンバーに対しては他とかぶらない番号にします。
    名前のあるメンバーに対してはすでに同名のメンバーが過去に存在していればそのときと同じ番号を再利用します。

## 切断

* クライアントが切断した場合、そのクライアントへの call (81) に対する応答が完了していないものがもしあれば、
call response (82) や call result (83) をそのクライアントに代わってcall呼び出し元に送ります。

## ping

* 一定時間おきに全クライアントに ping (89) を送信し、 ping (89) が返ってくるまでの時間を計測します。
* ping status req (91) を送ってきたメンバーに対しては、全クライアントのping情報を ping status (90) として一定時間おきに送ります。

## データの送信 (Sync)

* ToDo <del>飽きた</del>あとで続き書く

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [8-2. Client Spec](82_client_spec.md) | [8-4. Versioning](84_versioning.md) |

</div>
