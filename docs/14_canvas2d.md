# Canvas2D

\tableofcontents
\since
<span class="since-c">1.6</span>
<span class="since-js">1.4</span>
\sa
* C++ webcface::Canvas2D
* JavaScript [Canvas2D](https://na-trium-144.github.io/webcface-js/classes/Canvas2D.html)
(受信機能のみ)
* Python 未実装 <!--[webcface.Canvas3D](https://na-trium-144.github.io/webcface-python/webcface.canvas3d.html#webcface.canvas3d.Canvas3D)-->


## Point, Transform

WebCFaceでは座標を表すクラスとして webcface::Point, 座標変換(平行移動してから回転)を表すクラスとして webcface::Transform があります。

Pointでは x, y 座標、Transformでは回転角(radianで、 (x, y) = (1, 0) から (0, 1) に回る向きが正)を指定します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Point, Transformオブジェクトからは`pos()`, `rot()`で座標と回転角を取得できます。
    ```cpp
    Point p{1, 2};
    std::cout << p.pos(0); // 1 (x座標)
    Transform r{{1, 2}, std::numbers::pi / 2};
    std::cout << r.pos(0); // 1 (x座標)
    std::cout << r.rot(0); // pi / 2 (z軸回り)
    ```
    webcface::identity() は原点、回転なしのTransformを返します。

- <b class="tab-title">JavaScript</b>
    [Point](https://na-trium-144.github.io/webcface-js/classes/Point.html), [Transform](https://na-trium-144.github.io/webcface-js/classes/Transform.html) オブジェクトからは`pos`, `rot`で座標と回転角を取得できます。
    ```ts
    import { Point, Transform } from "webcface";
    const p = new Point([1, 2]);
    console.log(p.pos[0]); // 1 (x座標)
    const r = new Transform([1, 2], Math.PI / 2);
    console.log(r.pos[0]); // 1 (x座標)
    console.log(r.rot[0]); // pi / 2 (z軸回り)
    ```

</div>

## 送信

使い方は[View](13_view.md)とだいたい同じになっています。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::canvas2d からCanvas2Dオブジェクトを作り、
    Canvas2D::init() でCanvasのサイズを指定し、
    Canvas2D::add() で要素を追加し、
    最後にCanvas2D::sync()をしてからClient::sync()をすることで送信されます。

    例 (src/example/main.cc を参照)
    ```cpp
    webcface::Canvas2D canvas = wcli.canvas2D("canvas");
    canvas.init(100, 100);
    canvas.add(webcface::rect({10, 10}, {90, 90}),
               webcface::ViewColor::black);
    canvas.add(webcface::circle(webcface::Point{50, 50}, 20),
               webcface::ViewColor::red);
    webcface::Transform pos{ ... };
    canvas.add(webcface::polygon(
                   {{0, -5}, {-5, 0}, {-5, 10}, {5, 10}, {5, 0}}),
               pos,
               webcface::ViewColor::black, webcface::ViewColor::yellow,
               2);
    // ... 省略
    canvas.sync(); // ここまでにcanvasに追加したものをクライアントに反映
    wcli.sync();
    ```
    ![tutorial_canvas2d.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/tutorial_canvas2d.png)

    C++ではCanvas2Dのデストラクタでも自動的にCanvas2D::sync()が呼ばれます。

    \note
    Canvas2Dオブジェクトをコピーした場合、Canvas2Dオブジェクトの内容はコピーされるのではなく共有され、そのすべてのコピーが破棄されるまでsync()は呼ばれません。

    WebUIで表示するときには、initで指定したサイズの中で図を描画したものが画面の大きさに合わせて拡大縮小されます。

</div>

\note
Viewと同様、Canvas3Dの2回目以降の送信時にはWebCFace内部では前回からの差分のみが送信されます

### Geometry (2次元)

2次元の図形を表示するにはGeometryを指定します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    ```cpp
    using namespace webcface::Geometries;
    canvas.add(
        rect({0, 0}, {100, 100}),
        webcface::identity(),
        webcface::ViewColor::gray,
        webcface::ViewColor::white,
        1
    );
    ```
    addの引数に表示したいgeometryと、表示する位置の平行移動or回転、枠線の色、塗りつぶしの色、枠線の太さを指定します。
    詳細は webcface::Canvas2D::add を参照してください
    
    C++ではGeometryは webcface::Geometries 名前空間に定義されていますが、`webcface::` の名前空間でもアクセス可能です。

</div>

座標系は 右方向がx座標正、下方向がy座標正です。
(したがって回転角は右回りが正になります。)

Geometryは以下のものが用意されています。

### Line
指定した2点間に直線を引きます
```cpp
line(Point begin, Point end)
```

### Rect
指定した2点を頂点としx軸,y軸に平行な辺をもつ長方形を描画します
```cpp
rect(Point vertex1, Point vertex2)
```
またはoriginを中心として幅(x方向の長さ)がwidth, 高さ(y方向の長さ)がheightの長方形を描画します
```cpp
rect(Point origin, double width, double height)
```

### Circle
originを中心として半径radiusの円を描画します
```cpp
circle(Point origin, double radius)
```

### Polygon
指定した点をつなげた図形を描画します
```cpp
polygon(std::vector<Point> points)
```

## 受信
Viewなどと同様、Member::canvas2D() でCanvas2Dクラスのオブジェクトが得られ、
Canvas3D::tryGet() で値のリクエストをするとともに受信した値を取得できます。

Canvas2Dデータは
webcface::Canvas2DComponent
(JavaScript [Canvas2DComponent](https://na-trium-144.github.io/webcface-js/classes/Canvas3DComponent.html))
<!--Python [webcface.ViewComponent](https://na-trium-144.github.io/webcface-python/webcface.view.html#webcface.view.ViewComponent))-->
のリストとして得られ、
Canvas2DComponentオブジェクトから各種プロパティを取得できます。

### 時刻

Canvas2D::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

<!--\note Pythonでは Member.sync_time()-->

### Entry

Member::canvas2DEntries() でそのMemberが送信しているCanvas2Dのリストが得られます

また、Member::onCanvas2DEntry() で新しくデータが追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [View](13_view.md) | [Image](15_image.md) |

</div>
