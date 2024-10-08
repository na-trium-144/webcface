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

WebCFaceでは3次元の座標を webcface::Point, 座標変換(平行移動してから回転)を webcface::Transform で表せます([Canvas2D](./61_canvas2d.md)での2次元の座標を表すクラスと共通です)。

Pointでは x, y, z 座標、Transformでは x, y, z 座標と z, y, x軸回りの回転角を指定します。
(WebCFaceでは3次元の回転はz-y-xの順で回転するオイラー角で表現します。)

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

- <b class="tab-title">Python</b>
    [webcface.Point](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.Point), [webcface.Transform](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.Transform) オブジェクトからは`pos`, `rot`で座標と回転角を取得できます。
    ```py
    p = webcface.Point([1, 2, 3])
    print(p.pos[0]) # 1 (x座標)
    r = webcface.Transform([1, 2, 3], [math.pi / 2, 0, 0])
    print(r.pos[0]) # 1 (x座標)
    print(r.rot[0]) # pi / 2 (z軸回り)
    ```
    webcface.identity() は原点、回転なしのTransformを返します。

    Point同士は加算、減算、`==`, `!=`での比較ができます。
    また、int,floatと乗算、除算ができます。

    引数にPointやTransformをとる関数では、webcface.Point に変換することなく
    `[1, 2, 3]`のようなリストのままでも使えるものもあります。

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

    `webcface::Geometries` 名前空間に定義されています。
    ```cpp
    using namespace webcface::Geometries;
    ```
    をすると便利かもしれません
    \note namespace Geometries はinlineなので、 `webcface::` の名前空間でもアクセス可能です

    各要素はそれぞれの関数から webcface::Canvas3DComponent または webcface::TemporalComponent のオブジェクトとして得られます。
    `box(...).color(...)` などのようにメソッドチェーンすることで各要素にオプションを設定できます。

    色、表示位置<!-- 、クリック時に実行する関数 -->などを設定できます。
    使用可能なオプションは webcface::Canvas3DComponent のそれぞれのメソッドの説明を参照してください。
    <!-- 関数の実行については[Func](./53_func.md)も参照してください -->


- <b class="tab-title">JavaScript</b>
    JavaScriptでは [`geometries`](https://na-trium-144.github.io/webcface-js/variables/geometries.html) オブジェクト内にそれぞれの要素を表す関数があります
    ```ts
    import { geometries } from "webcface";
    ```
    オプションはそれぞれ関数の引数にオブジェクトで渡すことができます。

- <b class="tab-title">Python</b>
    Pythonでは [`webcface.geometries`](https://na-trium-144.github.io/webcface-python/webcface.geometries.html) モジュール内にあり、
    ```python
    from webcface.geometries import *
    ```
    とすることもできます

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
webcface内部では2次元の Rect と同じ扱いです

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

<!-- ### 時刻

~~Canvas3D::time()~~ でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。  
<span class="since-c">1.7</span>
<span class="since-js">1.6</span>
<span class="since-py"></span>
Member::syncTime() に変更
 -->
### Entry, Event

いずれも使い方は [Value](./51_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [6-2. Image](62_image.md) | [6-4. RobotModel](64_robot_model.md) |

</div>
