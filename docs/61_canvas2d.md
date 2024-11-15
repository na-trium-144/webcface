# 6-1. Canvas2D

\tableofcontents
\since
<span class="since-c">1.6</span>
<span class="since-js">1.4</span>
<span class="since-py">1.1</span>
\sa
* C++ webcface::Canvas2D (`webcface/canvas2d.h`)
* JavaScript [Canvas2D](https://na-trium-144.github.io/webcface-js/classes/Canvas2D.html)
* Python [webcface.Canvas2D](https://na-trium-144.github.io/webcface-python/webcface.canvas2d.html#webcface.canvas2d.Canvas2D)

2Dの図形の描画データを送受信する型です。

## Point, Transform

WebCFaceでは座標を表すクラスとして webcface::Point, 座標変換(平行移動してから回転)を表すクラスとして webcface::Transform があります。

Pointでは x, y 座標、Transformでは回転角(radianで、 (x, y) = (1, 0) から (0, 1) に回る向きが正)を指定します。

<div class="tabbed">

- <b class="tab-title">C++</b>
    Point, Transformオブジェクトからは`pos()`, <del>`rot()`</del> <span class="since-c">2.5</span> `rot2D()` で座標と回転角を取得できます。
    ```cpp
    Point p{1, 2};
    std::cout << p.pos(0) << ", " << p.pos(1); // 1, 2 (x, y 座標)
    Transform r{{1, 2}, std::numbers::pi / 2};
    std::cout << r.pos(0) << ", " << p.pos(1); // 1, 2 (x, y 座標)
    std::cout << r.rot2D(); // pi / 2
    ```
    
    * webcface::identity() は原点、回転なしのTransformを返します。
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
    const p = new Point([1, 2]);
    console.log(p.pos[0]); // 1 (x座標)
    const r = new Transform([1, 2], Math.PI / 2);
    console.log(r.pos[0]); // 1 (x座標)
    console.log(r.rot[0]); // pi / 2 (z軸回り)
    ```

- <b class="tab-title">Python</b>
    [webcface.Point](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.Point), [webcface.Transform](https://na-trium-144.github.io/webcface-python/webcface.transform.html#webcface.transform.Transform) オブジェクトからは`pos`, <del>`rot`</del> <span class="since-py">3.0</span> `rot_2d()` で座標と回転角を取得できます。
    ```py
    p = webcface.Point([1, 2])
    print(p.pos) # (1, 2) (x, y 座標)
    r = webcface.Transform([1, 2], math.pi / 2)
    print(r.pos) # (1, 2) (x, y 座標)
    print(r.rot_2d()) # pi / 2
    ```
    
    * webcface.identity() は原点、回転なしのTransformを返します。
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

\note
Viewと同様、Canvas3Dの2回目以降の送信時にはWebCFace内部では前回からの差分のみが送信されます

<div class="tabbed">

- <b class="tab-title">C++</b>
    Client::canvas2d からCanvas2Dオブジェクトを作り、
    Canvas2D::init() でCanvasのサイズを指定し、
    [View](./54_view.md)と同様に Canvas2D::add() または operator<< で要素を追加し、
    最後にCanvas2D::sync()をしてからClient::sync()をすることで送信されます。

    \note <span class="since-c">1.9</span> add関数の仕様を変更し << 演算子も実装して、Viewと同じ使い方になりました

    例 (src/example/main.cc も参照)
    ```cpp
    webcface::Canvas2D canvas = wcli.canvas2D("canvas");
    canvas.init(100, 100);
    canvas << webcface::rect({10, 10}, {90, 90})
                .color(webcface::ViewColor::black)
           << webcface::circle(webcface::Point{50, 50}, 20)
                .color(webcface::ViewColor::red);
    webcface::Transform pos{ ... };
    canvas << webcface::polygon({{0, -5}, {-5, 0}, {-5, 10}, {5, 10}, {5, 0}})
                .origin(pos)
                .color(webcface::ViewColor::black)
                .fillColor(webcface::ViewColor::yellow)
                .strokeWidth(2);
    // ... 省略
    canvas.sync(); // ここまでにcanvasに追加したものをクライアントに反映
    wcli.sync();
    ```
    ![example_canvas2d.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_canvas2d.png)

    C++ではCanvas2Dのデストラクタでも自動的にCanvas2D::sync()が呼ばれます。

    \note
    Canvas2Dオブジェクトをコピーした場合、Canvas2Dオブジェクトの内容はコピーされるのではなく共有され、そのすべてのコピーが破棄されるまでsync()は呼ばれません。

    WebUIで表示するときには、initで指定したサイズの中で図を描画したものが画面の大きさに合わせて拡大縮小されます。

- <b class="tab-title">JavaScript</b>
    \since <span class="since-js">1.5</span>
    
    Client.canvas2D からCanvas2Dオブジェクトを作り、
    set()の引数に要素をまとめてセットして使います。

    例
    ```ts
    const pos = new Transform(...);
    wcli.canvas2D("canvas").set(100, 100, [
        geometries.rect(new Point(10, 10), new Point(90, 90), {
            color: viewColor.black,
        }),
        geometries.circle(new Point(50, 50), 20, {
            color: viewColor.red,
        }),
        geometries.polygon([...], {
            origin: pos,
            color: viewColor.black,
            fillColor: viewColor.yellow,
            strokeWidth: 2,
        }),
    ]);
    wcli.sync();
    ```
    ![example_canvas2d.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_canvas2d.png)

    WebUIで表示するときには、initで指定したサイズの中で図を描画したものが画面の大きさに合わせて拡大縮小されます。

- <b class="tab-title">Python</b>
    Client.canvas2d からCanvas2Dオブジェクトを作り、
    Canvas2D.init() でCanvasのサイズを指定し、
    Canvas2D.add() で要素を追加し、
    最後にCanvas2D.sync()をしてからClient.sync()をすることで送信されます。

    例
    ```py
    canvas = wcli.canvas2d("canvas")
    canvas.init(100, 100)
    canvas.add(
        webcface.geometries.rect(webcface.Point(10, 10), webcface.Point(90, 90)),
        color=webcface.ViewColor.BLACK,
    )
    canvas.add(
        webcface.geometries.circle(webcface.Transform([50, 50], 0), 20),
        color=webcface.ViewColor.RED,
    )
    pos = webcface.Transform(...)
    canvas.add(
        webcface.geometries.polygon([[0, -5], [-5, 0], [-5, 10], [5, 10], [5, 0]]),
        pos,
        color=webcface.ViewColor.BLACK,
        fill=webcface.ViewColor.YELLOW,
        stroke_width=2
    )
    # ... 省略
    canvas.sync() # ここまでにcanvasに追加したものをクライアントに反映
    wcli.sync()#
    ```
    ![example_canvas2d.png](https://github.com/na-trium-144/webcface/raw/main/docs/images/example_canvas2d.png)

    init() で指定するキャンバスのサイズを canvas2d() 時に指定することもできます。(init() は不要になります)
    ```py
    canvas = wcli.canvas2d("canvas", 100, 100)
    ```
    また、with構文を使って `with wcli.canvas2d("hoge", width, height) as canvas:` などとするとwithを抜けるときに自動でcanvas.sync()がされます。

    WebUIで表示するときには、initで指定したサイズの中で図を描画したものが画面の大きさに合わせて拡大縮小されます。

</div>

### Geometry (2次元)

2次元の図形を表示するにはGeometryを指定します。

座標系は 右方向がx座標正、下方向がy座標正です。
(したがって回転角は右回りが正になります。)

描画したものが重なる場合、後にaddした要素が上に描画されます。

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">1.9</span>

    `webcface::geometries` 名前空間に定義されています。
    ```cpp
    using namespace webcface::geometries;
    ```
    をすると便利かもしれません
    \note namespace geometries はinlineなので、 `webcface::` の名前空間でもアクセス可能です

    各要素はそれぞれの関数から webcface::Canvas2DComponent または webcface::TemporalComponent のオブジェクトとして得られます。

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

<details><summary>ver1.8以前のC++</summary>

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

</details>

#### Line
指定した2点間に直線を引きます
```cpp
line(Point begin, Point end)
```

#### Rect
指定した2点を頂点としx軸,y軸に平行な辺をもつ長方形を描画します
```cpp
rect(Point vertex1, Point vertex2)
```
またはoriginを中心として幅(x方向の長さ)がwidth, 高さ(y方向の長さ)がheightの長方形を描画します
```cpp
rect(Point origin, double width, double height)
```

#### Circle
originを中心として半径radiusの円を描画します
```cpp
circle(Point origin, double radius)
```

<del>PythonではCanvas3Dとの兼ね合いで第1引数はTransform(使いにくいのでなんとかならないか?)</del>
<span class="since-py">3.0</span> で修正

#### Polygon
指定した点をつなげた図形を描画します
```cpp
polygon(std::vector<Point> points)
```

### 文字列の表示

<div class="tabbed">

- <b class="tab-title">C++</b>
    \since <span class="since-c">1.9</span>

    `webcface::Components::text()` をCanvas2Dに追加することができます。
    `textColor()`, `textSize()` などのオプションが使えます。
    ```cpp
    canvas << webcface::text("Button")
                .origin({35, 45})
                .textColor(webcface::ViewColor::orange)
                .textSize(10);
    ```

</div>

### オプション

各要素には以下のオプションを指定することができます。
(要素の種類によっては効果がないものもあります)

* origin: 図形の位置を移動・回転します。
    * それぞれのgeometryに対しても位置を指定することができますが、
    それに加えてoriginを指定した場合最終的な座標は origin \* 図形の座標 になります
* color: 図形の枠線の色を変更します。
* fillColor(C++), fill(Python): 背景色を変更します。
* strokeWidth: 線の太さを変更します。
    * 太さ1はCanvas2Dの座標系で1の長さ分の太さになります(拡大縮小で太さが変わる)
    * 指定しない場合0となります
        * WebUIではその場合Canvasの拡大に関係なく1ピクセルで表示します
* textSize: <span class="since-c">1.9</span> 文字のサイズ
    * 大きさ1は文字の高さがCanvas2Dの座標系で1の長さ分になります(拡大縮小で大きさが変わる)
* onClick: <span class="since-c">1.9</span> クリックされたときに実行する処理を指定できます。
    * Viewのbuttonと同様、 Funcオブジェクト、 <span class="since-c">2.5</span> FuncListerオブジェクト、 または関数を直接指定できます。
* id: <span class="since-c">2.5</span><span class="since-py">3.0</span> Viewのbuttonなどと同様です。
    * 指定する場合は一意な文字列を指定してください。

<div class="tabbed">

- <b class="tab-title">C++</b>
    `rect(...).color(...)` などのようにメソッドチェーンすることで各要素にオプションを設定できます。
    詳細は webcface::TemporalCanvas2DComponent のリファレンスを参照してください。

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
    `canvas2d.add(rect(...), color=...)` などのように、add() の引数にキーワード引数を渡すことでオプションを設定できます。

    色はViewと同様 ViewColor のenumで指定します。

</div>


## 受信
Viewなどと同様、Member::canvas2D() でCanvas2Dクラスのオブジェクトが得られ、
Canvas2D::tryGet() で値のリクエストをするとともに受信した値を取得できます。

Canvas2Dデータは
webcface::Canvas2DComponent
(JavaScript [Canvas2DComponent](https://na-trium-144.github.io/webcface-js/classes/Canvas3DComponent.html)),
Python [webcface.Canvas2DComponent](https://na-trium-144.github.io/webcface-python/webcface.canvas2d.html#webcface.canvas2d.Canvas2DComponent))
のリストとして得られ、
Canvas2DComponentオブジェクトから各種プロパティを取得できます。

また、Canvasの幅と高さも取得できます。

<!-- ### 時刻

~~Canvas2D::time()~~ でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。  
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
| [5-5. Log](55_log.md) | [6-2. Image](62_image.md) |

</div>
