# ゲームパッド

ブラウザに接続したゲームパッドの値をサーバーに送ることができます

## サーバー側
```cpp
WebCFace::setButtonName({"A", "B", ...});
WebCFace::setAxisName({"LStickX", "LStickY", ...});
```
のようにボタンと軸の名前を設定します

```cpp
WebCFace::GamepadState gamepad = WebCFace::getGamepad();
if(gamepad.connected){
	std::vector<bool> buttons = gamepad.buttons;
	std::vector<double> axes = gamepad.axes; // -1.0 〜 1.0
}
```
のように値を取得できます

## ブラウザ側

* ゲームパッドを接続するとサイドバーの「ゲームパッド」の下にゲームパッドが表示されます
	* 認識されない場合は任意のボタンを押すと認識するようになることがあります
	* あるいはブラウザを変える(chrome系の成功率が高め?)
* サイドバーからゲームパッドの画面を開きます
* サーバーの選択 で値を送信したいサーバーを選択します
* サーバー側で設定したボタン名が表示されるので ボタン名をクリック→対応するボタンを押す を繰り返してボタンの割当を設定します
* 設定できると以降そのボタンの状態がサーバーに送られます
* ブラウザをリロードしたり開き直すと以前の設定が自動で復元されます

