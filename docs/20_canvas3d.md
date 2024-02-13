# Canvas3D

\since
<span class="since-c">1.4</span>
<span class="since-js">1.3</span>
\sa
* C++ webcface::Canvas3D
* JavaScript [Canvas3D](https://na-trium-144.github.io/webcface-js/classes/Canvas3D.html)
* Python 未実装 <!--[webcface.Canvas3D](https://na-trium-144.github.io/webcface-python/webcface.canvas3d.html#webcface.canvas3d.Canvas3D)-->

3D空間上のオブジェクト配置データを送受信し、WebUI上に表示できます。

## Point, Transform

WebCFaceでは3次元の座標を表すクラスとして webcface::Point, 座標変換(平行移動してから回転)を表すクラスとして webcface::Transform があります。

Pointでは x, y, z 座標、Transformでは x, y, z 座標と z, y, x軸回りの回転角を指定します。
(WebCFaceでは回転はz-y-xの順で回転するオイラー角で表現します。)

<div class="tabbed">

- <b class="tab-title">C++</b>
    Point, Transformオブジェクトからは`pos()`, `rot()`で座標と回転角を取得できます。
    ```cpp
    Point p{1, 2, 3};
    std::cout << p.pos(0); // 1 (x座標)
    Transform r{1, 2, 3, std::numbers::pi / 2, 0, 0};
    std::cout << r.pos(0); // 1 (x座標)
    std::cout << r.rot(0); // pi / 2 (z軸回り)
    ```
    webcface::identity() は原点、回転なしのTransformを返します。

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

</div>

## 送信

使い方は[View](13_view.md)とだいたい同じになっています。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::canvas3d からCanvas3Dオブジェクトを作り、
    Canvas3D::add() で要素を追加し、
    最後にCanvas3D::sync()をしてからClient::sync()をすることで送信されます。
    ```cpp
    webcface::Canvas3D canvas = wcli.canvas3D("hoge");
    // canvas.init(); // ←オブジェクトcanvasを新規に構築せず繰り返し使いまわす場合は必要
    canvas.add(...);
    canvas.sync(); // ここまでにcanvasに追加したものをクライアントに反映
    wcli.sync();
    ```
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

</div>

Canvas3Dに追加できる要素として、Geometry(後述)、[RobotModel](./21_robot_model.md) があります。

### Geometry

3次元の図形を表示するにはGeometryを指定します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    using namespace webcface::Geometries;
    canvas.add(
        box(webcface::Point{0, 0, 0}, webcface::Point{2, 2, 2}),
        webcface::identity(),
        webcface::ViewColor::gray
    );
    ```
    addの引数に表示したいgeometryと、表示する位置、色を指定します。
    詳細は webcface::Canvas3D::add を参照してください
    
    C++ではGeometryは webcface::Geometries 名前空間に定義されていますが、`webcface::` の名前空間でもアクセス可能です。
- <b class="tab-title">JavaScript</b>
    ```ts
    import { geometries, viewColor, Point, Transform } from "webcface";
    canvas.set([
        [
            geometries.box(new Point(0, 0, 0), new Point(2, 2, 2)),
            new Transform(),
            viewColor.gray,
        ],
    ]);
    ```
    setの引数にgeometry、表示する位置(Transform)、色の3つをArrayにして指定します。

    Geometryは [geometries](https://na-trium-144.github.io/webcface-js/variables/geometries.html) に定義されています

</div>

Geometryは以下のものが用意されています。

### Line
指定した2点間に直線を引きます
```cpp
line(Point begin, Point end)
```

### Plane
originのxy平面上に、originを中心として幅(x方向の長さ)がwidth, 高さ(y方向の長さ)がheightの長方形を描画します
```cpp
plane(Transform origin, double width, double height)
```

### Box
指定した2点を頂点とし、各辺がx, y, z軸に平行な直方体を描画します
```cpp
box(Point vertex1, Point vertex2)
```

### Circle
originのxy平面上に、originを中心として半径radiusの円を描画します
```cpp
circle(Transform origin, double radius)
```

### Cylinder
originのyz平面上にoriginを中心として半径radiusの円を描画し、
それをx軸方向にlength押し出した円柱を描画します
```cpp
cylinder(Transform origin, double radius, double length)
```

### Sphere
originを中心とし半径radiusの球を描画します
```cpp
sphere(Point origin, double radius)
```

## 受信
Viewなどと同様、Member::canvas3D() でCanvas3Dクラスのオブジェクトが得られ、
Canvas3D::tryGet() で値のリクエストをするとともに受信した値を取得できます。

Canvas3Dデータは
webcface::Canvas3DComponent
(JavaScript [Canvas3DComponent](https://na-trium-144.github.io/webcface-js/classes/Canvas3DComponent.html))
<!--Python [webcface.ViewComponent](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.ViewComponent))-->
のリストとして得られ、
Canvas3DComponentオブジェクトから各種プロパティを取得できます。

### 時刻

Canvas3D::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

<!--\note Pythonでは Member.sync_time()-->

### Entry

Member::canvas3DEntries() でそのMemberが送信しているCanvas3Dのリストが得られます

また、Member::onCanvas3DEntry() で新しくデータが追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Image](15_image.md) | [RobotModel](21_robot_model.md) |

</div>
