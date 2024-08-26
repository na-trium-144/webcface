# 2-1. Server

\tableofcontents

WebCFaceを使用するときはserverを常時立ち上げておく必要があります。

## コマンドラインから

```sh
webcface-server
```
でサーバーを起動します。
デフォルトでは7530番ポートを開きクライアントの接続を待ちます。

* コマンドラインオプションで起動するポートを変更できたりします。詳細は`webcface-server -h`で確認してください

<span class="since-c">1.11</span>
TCPポートだけでなく、Unixドメインソケットも開くようになりました。
Linux,Macでは `/tmp/webcface/ポート番号.sock`
Windowsでは `C:\ProgramData\webcface\ポート番号.sock`
が自動的に作成されます。

さらに、WSL1上で起動した場合は `/mnt/c/ProgramData/webcface/ポート番号.sock` も追加で開きます。
これによりWindows側のクライアントがWSL1側のサーバーに接続することができます。

\note
標準エラー出力にログが出力されます。
デフォルトではクライアントの接続と切断がログに表示されます。  
<span class="since-c">1.1.7</span>
コマンドライン引数で `-v` を渡すとクライアントから送られてきたメッセージ、
`-vv` を渡すと送受信したメッセージすべてをログに表示します。
(ログが長く見づらくなります)

## サービスとして (Linuxのみ)

\since <span class="since-c">1.5.3</span>

配布しているdebパッケージでは [cmake/webcface-server.service](https://github.com/na-trium-144/webcface/blob/main/cmake/webcface-server.service) がインストールされ、
```sh
sudo systemctl enable webcface-server
sudo systemctl start webcface-server
```
でサーバーを常時自動起動させることができます。

