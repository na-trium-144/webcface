# starter

webcface-starterは設定ファイルにしたがってシェル関数としてコマンドの実行、停止ができるシンプルなWebCFaceバックエンドです。

systemdなどでwebcface-starterを自動起動するようにすると、設定したコマンドをWebCFaceのフロントエンドを使って起動させることができるようになります。

pipでインストールすると`webcface-starter`コマンドが /usr/local/bin にインストールされます。
```bash
webcface-starter /path/to/webcface-starter.toml
```
で起動できます。

## 設定ファイル
tomlファイルで設定します。ファイル名は任意です。
```toml
[init]
port = 80
name = "starter"

[[command]]
name = "cmake"
workdir = "/path/to/build"
exec = "cmake .."

[[command]]
name = "make"
workdir = "/path/to/build"
exec = "make -j7"

[[command]]
name = "main"
workdir = "/path/to/build"
exec = "./main"

[[server]]
addr = "172.17.0.2"
port = 80

[[server]]
addr = "172.17.0.2"
port = 3001
```

### init
* `port`: webcface-starterが起動するポートです。省略すると80になります
* `name`: webcface-starterのサーバー名です。省略すると「starter」になります
	* 複数のマシンのstarterに同時接続する場合などは名前を変えるといいかもしれません

### command
* `name`: コマンドの名前(フロントエンドでの表示名)
* `workdir`: コマンドを実行するディレクトリ 省略するとカレントディレクトリになります
* `exec`: 実行するコマンド
	* シェルで実行されます
	* `;`や`&&`などで複数コマンドを実行できます
	* `&`で並列実行できますが、`wait`を入れないと停止できなくなるかもしれない?
* フロントエンドでは「コマンド名.start」「コマンド名.terminate」「コマンド名.kill」の3つのシェル関数として表示されます
	* startで実行、terminateまたはkillで停止できます

### server
* 他のWebCFaceバックエンドにも接続したい場合ここに設定します(4-5節を参照)
* `addr`: アドレス 省略するとstarterと同じになります
* `port`: ポート


