# カスタムページ
C++側でページのレイアウトを定義し、フロントエンドにカスタムのページを表示することができます。
定義は初期化時の1回でよいです。

see also WebCFace::Layout

ヘッダーは`<webcface/layout.hpp>`

現在はver0.4以前の書き方もそのまま使えるようになっています
[ver0.4以前用](./2_5_custom_page_old.md)

## PageLayout
```cpp
WebCFace::PageLayout p("ページ名");
p.clear();
```
```py
p = webcface.PageLayout("ページ名")
p.clear()
```
のようにPageLayoutを定義すると、フロントエンドにはその名前のカスタムページが生成されます
* 同じ名前のPageLayoutを複数作ると、前のPageLayoutの末尾に追記されます
* clear()を呼ぶと以前のレイアウトがクリアされ上書きできます
* 毎周期PageLayoutを生成しclearすることで、動的にページを変化させることができます。

### 文字などの表示
```cpp
p << "hello, world!";
```
```py
p.add("hello, world!")
```
で「hello, world!」がカスタムページに表示されます。文字列、数値、bool値はこのように表示できます。

```cpp
p << 1 << 2 << 3;
```
```py
p.add(1).add(2).add(3)
```
のようにまとめて出力できます。ただしcoutと違ってそれぞれの間にはスペースが空きます

```cpp
p << p.endl;
```
```py
p.new_line()
```
で改行できます

### ボタンの表示
```cpp
p << WebCFace::Button("ボタンに表示する文字列", "関数名"_callback = 関数);
```
```py
p.add(webcface.Button("ボタンに表示する文字列", webcface.RegisterCallback("関数名", callback=関数)))
```
でボタンを表示できます。関数名は意味がないですが現状は他とかぶらない関数名をつけないとたぶんバグります。

```cpp
p << WebCFace::Button("ボタンに表示する文字列", "サーバー名:関数名"_callback);
```
```py
p.add(webcface.Button("ボタンに表示する文字列", webcface.RegisterCallback("サーバー名:関数名")))
```
で別サーバーの関数を実行するボタンができます。

```cpp
p << WebCFace::Button(...).MuiColor("色名");
```
```py
p.add(webcface.Button(..., mui_color="色名"))
```
で色を変更できます。
`primary`で青(デフォルト)、`secondary`で紫、`warning`で黄色、`error`で赤になります。

### Alert
文字を目立たせて表示することができます。
```cpp
p << WebCFace::Alert("文字列", "色名");
```
```py
p.add(webcface.Alert("文字列", "色名"))
```
色名はButtonと同じ

### Drawing
図などを描画する機能です
```cpp
auto d = WebCFace::Drawing(width, height);
auto l = d.createLayer("レイヤー名");
// 描画処理をする
p << d;
```
```py
d = webcface.Drawing(width, height)
l = d.create_layer("レイヤー名")
# 描画処理をする
p.add(d)
```
このようにDrawingを生成し、DrawingにLayerを作成し、Layerに対して描画処理をし、最後にPageLayoutにDrawingを追加することで表示できます
* Drawingで指定した幅と高さの範囲に描画でき、それが画面サイズに応じて拡大縮小されて表示されます
* xの方向は左から右, yの方向は上から下です

```cpp
d.addLayer("サーバー名:レイヤー名");
```
```py
d.add_layer("サーバー名:レイヤー名")
```
とすると別のDrawingに作成したレイヤーの内容をコピーしてくることができます

```cpp
l.drawLine(x1, y1, x2, y2, color);
l.drawRect(x1, y1, x2, y2, color);
l.drawCircle(x, y, r, color);
l.drawText(x, y, font_size, text, color);
```
```py
l.draw_line(x1, y1, x2, y2, color)
l.draw_rect(x1, y1, x2, y2, color)
l.draw_circle(x, y, r, color)
l.draw_text(x, y, font_size, text, color)
```
のようにそれぞれ直線、四角形塗りつぶし、丸塗りつぶし、テキストを描画できます

```cpp
l.drawRect(x1, y1, x2, y2, color).onClick("関数名"_callback = 関数);
```
```py
l.draw_rect(x1, y1, x2, y2, color).on_click(webcface.RegisterCallback("関数名", callback=関数))
```
のように描画した範囲をクリックしたときに関数を実行させることができます

## Dialog
```cpp
WebCFace::Dialog("ページ名");
WebCFace::Dialog(p);
```
を1度実行すると、カスタムページがブラウザのウィンドウ全面に表示されます

重要なメッセージに使います
