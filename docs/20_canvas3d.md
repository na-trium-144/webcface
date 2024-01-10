# Canvas3D

![c++ ver1.4](https://img.shields.io/badge/1.4~-00599c?logo=C%2B%2B)
![js ver1.3](https://img.shields.io/badge/1.3~-f7df1e?logo=JavaScript&logoColor=black)

API Reference → webcface::Canvas3D, webcface::Geometries, webcface::Point, webcface::Transform

3D空間上のオブジェクト配置データを送受信し、WebUI上に表示できます。

Member::canvas3D() でCanvas3Dクラスのオブジェクトが得られます
```cpp
webcface::Canvas3D hoge = wcli.member("a").canvas3D("hoge");
```

Member::canvas3DEntries() でそのMemberが送信しているCanvas3Dのリストが得られます
```cpp
for(const webcface::Canvas3D &v: wcli.member("a").canvas3DEntries()){
    // ...
}
```

Member::onCanvas3DEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onCanvas3DEntry().appendListener([](webcface::Canvas3D v){ /* ... */ });
```

## Point, Transform

WebCFaceでは3次元の座標を表すクラスとして webcface::Point, 座標変換(平行移動してから回転)を表すクラスとして webcface::Transform があります。

Pointでは x, y, z 座標、Transformでは x, y, z 座標と z, y, x軸回りの回転角を指定します。
(WebCFaceでは回転はz-y-xの順で回転するオイラー角で表現します。)

Point, Transformオブジェクトからは`pos()`, `rot()`で座標と回転角を取得できます。
```cpp
Point p{1, 2, 3};
double x = p.pos()[0]; // 1 (x座標)
Transform r{1, 2, 3, 4, 5, 6};
double x = r.pos()[0]; // 1 (x座標)
double alpha = r.rot()[0]; // 4 (z軸回り)
```

webcface::identity() は原点、回転なしのTransformを返します。

## 送信

使い方は[View](13_view.md)とだいたい同じになっています。
Canvas3Dオブジェクトを作り、
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
Canvas3Dオブジェクトをコピーした場合、Canvas3Dオブジェクトの内容はコピーされるのではなく共有され、そのすべてのコピーが破棄されるまでsync()は呼ばれません。

addに指定可能なものとして、Geometry(後述)、[RobotModel](./21_robot_model.md) があります。

### Geometry

3次元の図形を表示するにはGeometryを指定します。

```cpp
using namespace webcface::Geometries;
canvas.add(
    box(webcface::Point{0, 0, 0}, webcface::Point{2, 2, 2}),
    webcface::identity(),
    webcface::ViewColor::gray
);
```
* 最初の引数にGeometryを指定します。引数は種類ごとに異なります。(後述)
* 2番目の引数にTransformを指定するとGeometryを移動することができます。省略時はidentity()になります。
* 3番目の引数に色を指定します。指定方法はViewと同様の webcface::ViewColor です。省略時はinherit(表示上はgrayと同じ)になります。

C++ではGeometryは webcface::Geometries 名前空間に定義されていますが、`webcface::` の名前空間でもアクセス可能です。

以下のGeometryが利用可能です。

* `line(Point begin, Point end)`
    * beginからendまで直線を引く
* `plane(Transform origin, double width, double height)`
    * originのxy平面上に、originを中心として幅(x方向の長さ)がwidth, 高さ(y方向の長さ)がheightの長方形を描画
* `box(Point vertex1, Point vertex2)`
    * 対角の頂点2点を指定して直方体を描画
    * 各辺は x, y, z 軸に平行
* `circle(Transform origin, double radius)`
    * originのxy平面上に、originを中心として半径radiusの円を描画
* `cylinder(Transform origin, double radius, double length)`
    * originのyz平面上にoriginを中心として半径radiusの円を描画し、それをx軸方向にlength押し出した円柱を描画
* `sphere(Point origin, double radius)`
    * originを中心とし半径radiusの球を描画

## 受信
Canvas3D::tryGet(), get() で Canvas3DComponent のリストを取得できます。
Canvas3DComponent オブジェクトからGeometryなどパラメータを取得する方法に関しては webcface::Canvas3DComponent のリファレンスを参照してください。

Canvas3D::appendListener() で受信したデータが変化したときにコールバックを呼び出すことができます。
<!--(Pythonでは Canvas3D.signal)-->


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Image](15_image.md) | [RobotModel](21_robot_model.md) |

</div>
