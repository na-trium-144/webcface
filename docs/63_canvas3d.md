# 6-3. Canvas3D

\tableofcontents
\since
<span class="since-c">1.4</span>
<span class="since-js">1.3</span>
<span class="since-py">1.1</span>
\sa
* C++ webcface::Canvas3D (`webcface/canvas3d.h`)
* JavaScript [Canvas3D](https://na-trium-144.github.io/webcface-js/classes/Canvas3D.html)
* Python [webcface.Canvas3D](https://na-trium-144.github.io/webcface-python/webcface.canvas3d.html#webcface.canvas3d.Canvas3D)

3D空間上のオブジェクト配置データを送受信する型です。

## Point, Transform

WebCFaceでは3次元の座標を表すクラスとして webcface::Point, 座標変換(平行移動してから回転)を表すクラスとして webcface::Transform があります。
([Canvas2D](./61_canvas2d.md)での2次元の座標を表すクラスと共通です)。

\note
WebCFaceの内部では3次元の回転はz-y-xの順で回転するオイラー角で表現するのを標準としています。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Point, Transformオブジェクトからは`pos()`, <del>`rot()`</del> <span class="since-c">2.5</span> `rotEuler()` で座標と回転角を取得できます。
    ```cpp
    Point p{1, 2, 3};
    std::cout << p.pos(0) << ", " << p.pos(1) << ", " << p.pos(2); // 1, 2, 3 (x, y, z 座標)
    Transform r{{1, 2, 3}, rotFromEuler(std::numbers::pi / 2, 0, 0)};
    std::cout << r.pos(0) << ", " << r.pos(1) << ", " << r.pos(2); // 1, 2, 3 (x, y, z 座標)
    std::cout << r.rotEuler()[0]; // pi / 2 (z軸回り)
    ```
    * Transform のコンストラクタには、3次元の平行移動(x, y, zの値)と、
    3次元の回転を指定します。
        * <span class="since-c">2.5</span> 3次元の回転は webcface::rotX(), rotY(), rotZ(), rotFromEuler(), rotFromMatrix(), rotFromQuat(), rotFromAxisAngle() を使って指定します。
        * webcface::rotFromEuler() や Transform::rotEuler() ではデフォルトではz-y-xの順で回転する系のオイラー角 (webcface::AxisSequence::ZYX) を扱いますが、引数で別の系(全12種類)も指定可能です。
        詳細は AxisSequence の説明を参照
    * webcface::identity() は原点、回転なしのTransformを返します。
    * <span class="since-c">2.5</span> 回転なしで平行移動のみのTransformを得るには、 webcface::translate() を使ってください。
    平行移動なしで回転のみのTransformを得るには、 rotFrom〜 系の関数で得られる Rotation クラスがTransformの代わりとして使えます(またはTransformにキャストできます。)
    * <span class="since-c">2.5</span>
    Point同士は加算、減算ができます。
    また、実数の乗算、除算ができます。
    * <span class="since-c">2.5</span>
    Transform \* Transform, Transform \* Point の乗算ができます。
    右辺の座標系全体を、左辺の値の分だけ回転移動してから平行移動したものになります。
        * 同じ効果を持つ関数として Transform::appliedTo() があります。
    * <span class="since-c">2.5</span>
    Transform::inversed() で逆変換が得られます。
    * <span class="since-c">2.5</span> Point同士、Transform同士は `==`, `!=`での比較ができます。
        * 完全一致での判定ではなく、各要素の差がそれぞれ 1e-8 未満であればtrueになります。

- <b class="tab-title">JavaScript</b>
    [Point](https://na-trium-144.github.io/webcface-js/classes/Point.html), [Transform](https://na-trium-144.github.io/webcface-js/classes/Transform.html) オブジェクトからは`pos`, `rot`で座標と回転角を取得できます。
    ```ts
    import { Point, Transform } from "webcface";
    const p = new Point([1, 2, 3]);
    console.log(p.pos[0]); // 1 (x座標)
    const r = new Transform([1, 2, 3], [Math.PI / 2, 0, 0]);
    console.log(r.pos[0]); // 1 (x座標)
    console.log(r.rot[0]); // pi / 2 (z軸回り)
    ```
    また、Transform.rotMatrix で3x3の回転行列、Transform.tfMatrix で4x4の同次変換行列が得られます。

- <b class="tab-title">Python</b>
    [webcface.Point](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.Point), [webcface.Transform](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.Transform) オブジェクトからは`pos`, <del>`rot`</del> <span class="since-py">3.0</span> `rot_euler()` で座標と回転角を取得できます。
    ```py
    p = webcface.Point([1, 2, 3])
    print(p.pos) # (1, 2, 3) (x, y, z 座標)
    r = webcface.Transform([1, 2, 3], webcface.rot_from_euler([math.pi / 2, 0, 0]))
    print(p.pos) # (1, 2, 3) (x, y, z 座標)
    print(r.rot_euler()[0]) # pi / 2 (z軸回り)
    ```

    * Transform のコンストラクタには、3次元の平行移動(x, y, zの値)と、
    3次元の回転を指定します。
        * <span class="since-py">3.0</span> 3次元の回転は webcface.rot_x(), rot_y(), rot_z(), rot_from_euler(), rot_from_matrix(), rot_from_quat(), rot_from_axis_angle() を使って指定します。
        * webcface.rot_from_euler() や Transform.rot_euler() ではデフォルトではz-y-xの順で回転する系のオイラー角 (AxisSequence.ZYX) を扱いますが、引数で別の系(全12種類)も指定可能です。
        詳細は [webcface.AxisSequence](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.AxisSequence) の説明を参照
    * webcface.identity() は原点、回転なしのTransformを返します。
    * <span class="since-py">3.0</span> 回転なしで平行移動のみのTransformを得るには、 webcface.translate() を使ってください。
    平行移動なしで回転のみのTransformを得るには、 rot_from〜 系の関数で得られる Rotation クラスがTransformの代わりとして使えます
    * Point同士は加算、減算ができます。
    また、int,floatと乗算、除算ができます。
    * <span class="since-py">3.0</span>
    Transform \* Transform, Transform \* Point の乗算ができます。
    右辺の座標系全体を、左辺の値の分だけ回転移動してから平行移動したものになります。
        * 同じ効果を持つ関数として Transform.applied_to_point(), applied_to_transform() があります。
    * <span class="since-py">3.0</span>
    Transform.inversed() で逆変換が得られます。
    * Point同士、Transform同士は `==`, `!=`での比較ができます。
        * <span class="since-py">3.0</span> 完全一致での判定ではなく、各要素の差がそれぞれ 1e-8 未満であればtrueになります。
    * 引数にPointやTransformをとる関数では、webcface.Point に変換することなく
    `[1, 2]` や `[[1, 2], pi / 2]` のようなリストのままでも使えるものもあります。


</div>

## 送信

使い方は[View](54_view.md)とだいたい同じになっています。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::canvas3d からCanvas3Dオブジェクトを作り、
    [View](./54_view.md)と同様に Canvas3D::add() または operator<< で要素を追加し、
    最後にCanvas3D::sync()をしてからClient::sync()をすることで送信されます。

    \note <span class="since-c">1.9</span> add関数の仕様を変更し << 演算子も実装して、Viewと同じ使い方になりました

    例
    ```cpp
    webcface::Canvas3D world = wcli.canvas3D("omniwheel_world");
    // world.init(); // ←オブジェクトcanvasを新規に構築せず繰り返し使いまわす場合は必要
    world << webcface::plane(webcface::identity(), 3, 3)
                .color(webcface::ViewColor::white);
    world << webcface::box({-1.5, -1.5, 0}, {1.5, -1.5, 0.1})
                .color(webcface::ViewColor::gray);
    world << webcface::box({-1.5, 1.5, 0}, {1.5, 1.5, 0.1})
                .color(webcface::ViewColor::gray);
    world << webcface::box({-1.5, -1.5, 0}, {-1.5, 1.5, 0.1})
                .color(webcface::ViewColor::gray);
    world << webcface::box({1.5, -1.5, 0}, {1.5, 1.5, 0.1})
                .color(webcface::ViewColor::gray);
    // RobotModel のドキュメントを参照
    world << wcli.robotModel("omniwheel")
                .origin(...)
                .angle("line_rotation", -i);
    world.sync(); // ここまでにcanvasに追加したものをクライアントに反映
    wcli.sync();
    ```
    ![example_wheel.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_wheel.png)

    C++ではCanvas3Dのデストラクタでも自動的にCanvas3D::sync()が呼ばれます。

    \note
    Canvas3Dオブジェクトをコピーした場合、Canvas3Dオブジェクトの内容はコピーされるのではなく共有され、そのすべてのコピーが破棄されるまでsync()は呼ばれません。

- <b class="tab-title">JavaScript</b>
    Client::canvas3D からCanvas3Dオブジェクトを作り、
    set()の引数に要素をまとめてセットして使います。
    ```ts
    wcli.canvas3D("hoge").set([
        ...
    ]);
    ```

- <b class="tab-title">Python</b>
    Client.canvas3d からCanvas3Dオブジェクトを作り、
    Canvas3D.add() で要素を追加し、
    最後にCanvas3D.sync()をしてからClient.sync()をすることで送信されます。

    with構文を使って `with wcli.canvas3d("hoge") as canvas:` などとするとwithを抜けるときに自動でcanvas.sync()がされます。

</div>

\note
Viewと同様、Canvas3Dの2回目以降の送信時にはWebCFace内部では前回からの差分のみが送信されます

Canvas3Dに追加できる要素として、Geometry(後述)、[RobotModel](./64_robot_model.md) があります。

### Geometry (3次元)

3次元の図形を表示するにはGeometryを指定します。
一部の図形は2次元のGeometryと共通のインタフェースになっています。

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">1.9</span>

    `webcface::geometries` 名前空間に定義されています。
    ```cpp
    using namespace webcface::geometries;
    ```
    をすると便利かもしれません
    \note namespace geometries はinlineなので、 `webcface::` の名前空間でもアクセス可能です

    各要素はそれぞれの関数から webcface::Canvas3DComponent または webcface::TemporalComponent のオブジェクトとして得られます。

- <b class="tab-title">JavaScript</b>
    JavaScriptでは [`geometries`](https://na-trium-144.github.io/webcface-js/variables/geometries.html) オブジェクト内にそれぞれの要素を表す関数があります
    ```ts
    import { geometries } from "webcface";
    ```

- <b class="tab-title">Python</b>
    Pythonでは [`webcface.geometries`](https://na-trium-144.github.io/webcface-python/webcface.geometries.html) モジュール内にあり、
    ```python
    from webcface.geometries import *
    ```
    とすることもできます

</div>

#### Line
指定した2点間に直線を引きます
```cpp
line(Point begin, Point end)
```

#### Plane
originのxy平面上に、originを中心として幅(x方向の長さ)がwidth, 高さ(y方向の長さ)がheightの長方形を描画します
```cpp
plane(Transform origin, double width, double height)
```
webcface内部では2次元の Rect と同じ扱いです

#### Box
指定した2点を頂点とし、各辺がx, y, z軸に平行な直方体を描画します
```cpp
box(Point vertex1, Point vertex2)
```

#### Circle
originのxy平面上に、originを中心として半径radiusの円を描画します
```cpp
circle(Transform origin, double radius)
```

#### Cylinder
originのyz平面上にoriginを中心として半径radiusの円を描画し、
それをx軸方向にlength押し出した円柱を描画します
```cpp
cylinder(Transform origin, double radius, double length)
```

#### Sphere
originを中心とし半径radiusの球を描画します
```cpp
sphere(Point origin, double radius)
```

### オプション

各要素には以下のオプションを指定することができます。
(要素の種類によっては効果がないものもあります)

* origin: 図形の位置を移動・回転します。
    * それぞれのgeometryに対しても位置を指定することができますが、
    それに加えてoriginを指定した場合最終的な座標は origin \* 図形の座標 になります
* color: 図形の枠線の色を変更します。
* id: <span class="since-c">2.5</span><span class="since-py">3.0</span> Viewのbuttonなどと同様です。
    * 指定する場合は一意な文字列を指定してください。

<div class="tabbed">

- <b class="tab-title">C++</b>
    `rect(...).color(...)` などのようにメソッドチェーンすることで各要素にオプションを設定できます。
    詳細は webcface::TemporalCanvas3DComponent のリファレンスを参照してください。

    色はViewと同様 webcface::ViewColor のenumで指定します。

<!-- 
- <b class="tab-title">C</b>
    wcfViewComponent 構造体のメンバーでオプションを指定することができます。

    ```c
    wcfViewComponent vc[10];
    vc[0] = wcfText("hello world\n");
    vc[0].text_color = WCF_COLOR_RED;
    ```
 -->

- <b class="tab-title">JavaScript</b>
    `rect(..., { color: ..., }))`
    などのように、オプションはそれぞれ関数の引数にオブジェクトで渡すことができます。

- <b class="tab-title">Python</b>
    `canvas3d.add(box(...), color=...)` などのように、add() の引数にキーワード引数を渡すことでオプションを設定できます。

    色はViewと同様 ViewColor のenumで指定します。

</div>

## 受信
Viewなどと同様、Member::canvas3D() でCanvas3Dクラスのオブジェクトが得られ、
Canvas3D::tryGet() で値のリクエストをするとともに受信した値を取得できます。

Canvas3Dデータは
webcface::Canvas3DComponent
(JavaScript [Canvas3DComponent](https://na-trium-144.github.io/webcface-js/classes/Canvas3DComponent.html))
<!--Python [webcface.ViewComponent](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.ViewComponent))-->
のリストとして得られ、
Canvas3DComponentオブジェクトから各種プロパティを取得できます。

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [6-2. Image](62_image.md) | [6-4. RobotModel](64_robot_model.md) |

</div>
