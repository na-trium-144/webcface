# 7-7. webcface-joystick

\tableofcontents
\since tools ver2.3

PCに接続されたJoystickの値を読み、WebCFaceに送信します。
コマンドラインだけでなく、WebUIから送信する機能もあります。

## コマンドライン引数
```
Usage: webcface-joystick [OPTIONS] [ID] [index]
```
* `-h`: ヘルプを表示します。
* `-a address`: 接続するサーバーのアドレスです。省略時は127.0.0.1になります。
* `-p port`: 接続するサーバーのポートです。省略時は7530になります。
* `-m name`: データを送信するメンバー名です。省略時は `webcface-joystick` になります。
* `ID`, `index`: 使用するJoystickを指定します。
* `-l`: 接続されているJoystickの一覧を表示します。

### 使い方

まず `webcface-joystick -l` で接続されているJoystickを表示します。
```
ID        Type                         Name
2563-0575 PS3                          SHANWAN 2In1 USB Joystick
```

そして、表示されたID (この例では `2563-0575`) を引数に渡して起動すると (`webcface-joystick 2563-0575`)、
Joystickのボタンやスティックなどの値がリアルタイムにWebCFaceに送られます。
```
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Name: SHANWAN 2In1 USB Joystick
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Type: PS3
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Avaliable game_buttons: a, b, x, y, back, guide, start, leftstick, rightstick, leftshoulder, rightshoulder, dpup, dpdown, dpleft, dpright
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Avaliable game_axes: leftx, lefty, rightx, righty, lefttrigger, righttrigger
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Number of buttons: 13
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Number of axes: 4
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Number of hats: 4
[2025-02-09 16:28:19.519] [webcface-joystick] [info] Number of balls: 0
```

![joystick_buttons](https://github.com/na-trium-144/webcface/raw/main/docs/images/joystick_buttons.png)

\note
* デバイスが1つしかない場合IDの指定を省略し `webcface-joystick` だけでも起動することができます。
* 同じIDのデバイスが複数存在する場合は、`2563-0575 0` `2563-0575 1` ... のように指定してください。

デフォルトでは `webcface-joystick` という名前のmemberとしてデータを送信します。
複数起動して複数のデバイスの値を送信したい場合、名前がかぶらないよう `-m` オプションで指定してください。

送信されるデータの名前は `buttons` `axes` `hats` `balls` で、それぞれ Value 型 (数値の配列型) です。
特に `axes` についてはそれぞれ -1 〜 1 の値になります。
Joystickのどのボタンが配列のどの要素に割り当てられるかはデバイスやOSによって異なる場合があるので、実際のデバイスで確認してください。

また、`webcface-joystick` コマンドのログに `Avaliable game_buttons:` のような表示がある場合、
SDL2 ライブラリの機能を使ってボタンの名前などを取得することができたことを示しています。
これが表示されるかどうかはデバイスに依存します。
この場合、ログに表示されているそれぞれのボタン名について
`game_buttons.a` のような名前の Value データ (これは配列ではなく単一の値) としても値を取得することができます。

## WebUIから送信する (webui-joystick)
\since webui ver1.14.0

ブラウザでWebUIを開いている端末 (PC、タブレット、スマートフォンなど) に接続されているコントローラーの値を Gamepad API を通して取得しWebCFaceに送信することができます。

コントローラーがWebUIに認識されるとメニューにコントローラーのアイコンが表示され、
それを開くとコントローラーの情報が表示されます。
(コントローラーが認識されるためにはWebUIを開いた状態でコントローラーのどれかのボタンを押す必要がある場合があります。)

![joystick_webui_api](https://github.com/na-trium-144/webcface/raw/main/docs/images/joystick_webui_api.png)
![joystick_webui_menu](https://github.com/na-trium-144/webcface/raw/main/docs/images/joystick_webui_menu.png)

「送信するMember名」を指定し、その左のスイッチをオンにすることで、
`webcface-joystick` と同様のデータ形式でコントローラーの値が送信されます。
対応しているデータは `buttons` `axes` のみです。
コントローラーのどのボタンが配列のどの要素に割り当てられるかはデバイスやOSによって異なる場合があるので、実際のデバイスで確認してください。

Member名はデフォルトでは「webui-joystick-(ランダムな4桁の値)-0」となっています。
ランダムな4桁の値 の部分はその端末上のそのブラウザではタブを開き直したり再起動しても常に一定です。
コントローラーを複数接続すると末尾の -0 の部分が変わります。
Member名を手動で変更した場合はそれがブラウザに保存され、ブラウザを開き直しても変更後の名前が使われます。

## 受信プログラム例

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    webcface::Member joystick = wcli.member("webui-joystick-2192-0");
    
    if(joystick.connected()){
        bool circle_pressed = joystick.value("buttons")[1].get();
        bool cross_pressed = joystick.value("buttons")[2].get();
        double lstick_x = joystick.value("axes")[0].get(); // -1 〜 1
        double lstick_y = joystick.value("axes")[1].get();
    }
    ```

</div>



<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [7-6. webcface-notepad](76_notepad.md) | [8-1. Message](81_message.md) |

</div>
