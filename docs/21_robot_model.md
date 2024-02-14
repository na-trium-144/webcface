# RobotModel

\since
<span class="since-c">1.4</span>
<span class="since-js">1.3</span>
\sa
* C++ webcface::RobotModel
* JavaScript [RobotModel](https://na-trium-144.github.io/webcface-js/classes/RobotModel.html)
(受信機能のみ)
* Python 未実装 <!--[webcface.Canvas3D](https://na-trium-144.github.io/webcface-python/webcface.canvas3d.html#webcface.canvas3d.Canvas3D)-->

RobotModelはリンク(link)と関節(joint)の構造を定義します。
[Canvas3D](20_canvas3d.md) 上に表示して関節を動かすことができます。

## 送信

モデルはRobotLinkのリストで表されます。
Client::robotModel からRobotModelオブジェクトを作り、 RobotModel::set() でリンクのリストをセットすることでモデルを定義し送信できます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    using namespace webcface::RobotJoints;
    using namespace webcface::Geometries;
    wcli.robotModel("hoge").set({
        webcface::RobotLink(
            "a",
            fixedAbsolute(),
            line(...),
            webcface::ViewColor::yellow
        ),
        webcface::RobotLink(
            "b",
            rotationalJoint("joint1", "a", webcface::Transform{...}, 0),
            line(...),
            webcface::ViewColor::red
        ),
    });
    ```

</div>

## RobotLink

RobotLinkのコンストラクタには
* リンクの名前
* 関節の情報 (RobotJoint) (省略可→fixedAbsolute())
* リンクの形状 (Geometry)
* リンクの色 (ViewColor) (省略可→gray)

を指定する必要があります。

Geometryについては[Canvas3D](20_canvas3d.md)、ViewColorについては[View](13_view.md)のページを参照してください。

## RobotJoint
RobotJointには以下の4種類があります。

C++では `webcface::RobotJoints` 名前空間に定義されていますが、`webcface::` の名前空間でもアクセス可能です。

### fixedAbsolute
* 親リンクを指定せず、絶対座標を指定して固定します。
* 引数のoriginを省略するとidentity()になります。
```cpp
fixedAbsolute(Transform origin)
```
### fixedJoint
* 固定された関節です。親リンクの名前と、親リンクからの相対座標を指定します。
```cpp
fixedJoint(std::string parent, Transform origin)
```
### rotationalJoint
* 回転する関節です。
* joint_nameは関節の名前、parentは親リンクの名前
* originは親リンクからの相対座標を指定します。originのz軸が回転軸となり、z軸正の方向から見て反時計回りが正になります。
* 初期状態(このモデルの定義通りの座標)の回転角をdefault_angleに指定します(省略時0)
    * 例えばここでdefault_angle=1を指定し、後でCanvas3Dへの表示時にangle=2を指定すると正方向に (2-1=) 1rad回転します
```cpp
rotationalJoint(std::string joint_name, std::string parent, Transform origin, double default_angle)
```
### prismaticJoint
* 直線運動する関節です。
* originのz軸が運動方向です。
```cpp
prismaticJoint(std::string joint_name, std::string parent, Transform origin, double default_angle)
```

## Canvas3Dに表示する


<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    canvas.add(
        wcli.robotModel("hoge"),
        webcface::identity(),
        {
            {"joint1", std::numbers::pi / 4},
            {...},
        },
    );
    ```
    Canvas3D::addの第1引数にRobotModel、第2引数にモデルの位置、第3引数にjointの回転角を指定してCanvas3Dにモデルを表示することができます。
    引数に渡す回転角の型は `std::unordered_map<std::string, double>` で指定します。
    \note
    RobotModelはこのプログラム内で定義したものではなく別Memberのものを指定することもできます。

    　<!-- -->

</div>

## 受信

Member::robotModel() でRobotModelクラスのオブジェクトが得られ、
Text::tryGet(), Text::tryGetRecurse() で値のリクエストをするとともに受信した値を取得できます。

RobotModelデータは
webcface::RobotLink
(JavaScript [RobotLink](https://na-trium-144.github.io/webcface-js/classes/RobotLink.html))
<!--Python [webcface.ViewComponent](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.ViewComponent))-->
のリストとして得られ、
RobotLinkオブジェクトから各種プロパティを取得できます。

### 時刻

RobotModel::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

<!--\note Pythonでは Member.sync_time()-->

### Entry

~~Member::robotModels() で~~ そのMemberが送信しているRobotModelのリストが得られます  
<span class="since-c">1.6</span>
Member::robotModelEntries() に変更

また、Member::onRobotModelEntry() で新しくデータが追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Canvas3D](20_canvas3d.md) | [Func](30_func.md) |

</div>
