# 2-1. Server

\tableofcontents

WebCFaceを使用するときはserverを常時立ち上げておく必要があります。

## コマンドラインから

```sh
webcface-server
```
でサーバーを起動します。
デフォルトでは7530番ポートを開きクライアントの接続を待ちます。

<span class="since-c">1.11</span>
TCPポートだけでなく、Unixドメインソケットも開くようになりました。
Linux,Macでは `/tmp/webcface/ポート番号.sock`
Windowsでは `C:\ProgramData\webcface\ポート番号.sock`
が自動的に作成されます。

さらに、WSL1上で起動した場合は `/mnt/c/ProgramData/webcface/ポート番号.sock` も追加で開きます。
これによりWindows側のクライアントがWSL1側のサーバーに接続することができます。

## コマンドライン引数

```
Usage: webcface-server [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -p,--port UINT              Server port (default: 7530)
  -v,--verbose [0]            Show all received messages (-vv to show sent messages too)
  -l,--keep-log INT           Number of lines of received log to keep (default: 1000)
                              (keep all log by setting -1)
```
* `-h`: ヘルプを表示します。
* `-p port`: サーバーを開くポートを変更できます。(デフォルト=7530)
* `-v`: (ver1.1.7から) サーバーが受信したメッセージを表示します。(デバッグ用)
    * デフォルトではクライアントが接続・切断したときにのみメッセージが表示されます。
    * `-vv` で送信したメッセージもすべてログ出力します。
* `-l lines`: [Log](./55_log.md)のデータをサーバーが保持しておく行数を変更できます。(デフォルト=1000)

## サービスとして (Linuxのみ)

\since <span class="since-c">1.5.3</span>

READMEの手順に従って [webcface-server.service](https://github.com/na-trium-144/webcface/blob/main/scripts/webcface-server.service) が /etc/systemd/system または /usr/lib/systemd/system にインストールされていれば、
```sh
sudo systemctl enable webcface-server
sudo systemctl start webcface-server
```
でサーバーを常時自動起動させることができます。

## WebCFace Desktop

コマンドラインを使うことなく、 WebCFace Desktop からサーバーを起動できます。
詳細は次のWebUIのページで

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [1-2. Tutorial (Communication)](12_tutorial_comm.md) | [2-2. WebUI](22_webui.md) |

</div>
