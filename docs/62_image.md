# 6-2. Image

\tableofcontents
\since
<span class="since-c">1.3</span>
<span class="since-js">1.2</span>
<span class="since-py">2.4</span>
\sa
* C++ webcface::Image (`webcface/image.h`)
* JavaScript [Image](https://na-trium-144.github.io/webcface-js/classes/Image.html)
* Python [webcface.Image](https://na-trium-144.github.io/webcface-python/webcface.image.html#webcface.image.Image)

画像データを送受信します。
画像のリサイズ、圧縮などの処理をサーバー側で行う機能があります。

<!--
## コマンドライン

```sh
webcface-cv-capture 0
```
などとするとOpenCVのVideoCaptureでキャプチャできるWebカメラなどの画像をwebcfaceに送信することができます。

詳細は [webcface-cv-capture](./72_cv_capture.md) のページを参照
-->

## 送信

<div class="tabbed">

- <b class="tab-title">C++</b>
    画像データを表す型として webcface::ImageFrame があります。
    生の画像データがあればImageFrameのコンストラクタに渡すことができます。
    ```cpp
    unsigned char data[640 * 480 * 3]; /* = {...}; */

    // dataの中身がImageFrameにコピーされる
    // sizeWH で幅と高さを指定、または sizeHW で高さと幅を指定
    webcface::ImageFrame frame(webcface::sizeWH(640, 480), data, webcface::ImageColorMode::rgb);

    // ver1.11以前: サイズは高さ,幅の順で指定
    // webcface::ImageFrame frame(480, 640, data, webcface::ImageColorMode::rgb);
    ```
    ImageColorModeとしては `gray`(8bitグレースケール)、`rgb`(8bit×3)、`bgr`(8bit×3)、`rgba`(8bit×4)、`bgra`(8bit×4) が扱えます。

    \note
    ImageFrameオブジェクト内部では画像データは`shared_ptr<vector<unsigned char>>`で保持されます。  
    ImageFrameのコンストラクタにdataのポインタを渡した場合、
    dataの内容(width × height × channels() バイト)がすべてコピーされます。  
    ImageFrameをコピーした場合は画像データが共有されます(コピーされません)。
    
    <span></span>

    Client::image からImageオブジェクトを作り、 Image::set() でImageFrame型の画像データを代入し、Client::sync()することで送信されます。
    set() の代わりに代入演算子(Image::operator=)でも同様のことができます。
    ```cpp
    wcli.image("hoge").set(frame);
    wcli.image("hoge") = frame;
    ```

- <b class="tab-title">JavaScript</b>
    画像データを表す型として [ImageFrame](https://na-trium-144.github.io/webcface-js/classes/ImageFrame.html) があります。
    生の画像データがあればImageFrameのコンストラクタに渡すことができます。
    ```ts
    import { ImageFrame, imageColorMode } from "webcface";
    const data: ArrayBuffer = /* 640 * 480 * 3 バイトのデータ... */;
    // 画像サイズは 横, 縦 で指定
    const frame: ImageFrame = new ImageFrame(640, 480, data, imageColorMode.rgb);
    ```
    imageColorModeとしては `gray`(8bitグレースケール)、`rgb`(8bit×3)、`bgr`(8bit✕3)、`rgba`(8bit✕4)、`bgra`(8bit✕4) が扱えます。

    Client.image からImageオブジェクトを作り、 Image.set() でImageFrame型の画像データを代入し、Client.sync()することで送信されます。
    ```cpp
    wcli.image("hoge").set(frame);
    ```

- <b class="tab-title">Python</b>
    画像データを表す型として [webcface.ImageFrame](https://na-trium-144.github.io/webcface-python/webcface.image_frame.html#webcface.image_frame.ImageFrame) があります。
    生の画像データがあればImageFrameのコンストラクタに渡すことができます。
    ```python
    from webcface import ImageFrame, ImageColorMode, ImageCompressMode
    # data: 640 * 480 * 3 バイトの bytes 型
    # 画像サイズは 横, 縦 で指定
    frame = ImageFrame(640, 480, data, ImageColorMode.RGB, ImageCompressMode.RAW)
    ```
    ImageColorModeとして `GRAY`(8bitグレースケール)、`RGB`(8bit×3)、`BGR`(8bit✕3)、`RGBA`(8bit✕4)、`BGRA`(8bit✕4) が扱えます。

    Client.image からImageオブジェクトを作り、 Image.set() でImageFrame型の画像データを代入し、Client.sync()することで送信されます。
    ```cpp
    wcli.image("hoge").set(frame)
    ```

</div>

### 外部ライブラリの利用

<div class="tabbed">

- <b class="tab-title">OpenCV (C++)</b>
    ImageFrame → cv::Mat
    ```cpp
    cv::Mat img_mat(img_frame.rows(), img_frame.cols(), CV_8UC3, img_frame.data().data());
    ```
    cv::Mat → ImageFrame
    ```cpp
    assert(img_mat.depth() == CV_8U);
    webcface::ImageFrame img_frame;
    // https://stackoverflow.com/questions/26681713/convert-mat-to-array-vector-in-opencv
    if (img_mat.isContinuous()) {
        img_frame = webcface::ImageFrame(webcface::sizeHW(img_mat.rows, img_mat.cols),
                                         img_mat.data, webcface::ImageColorMode::bgr);
    } else {
        img_frame = webcface::ImageFrame(webcface::sizeHW(img_mat.rows, img_mat.cols),
                                         webcface::ImageColorMode::bgr);
        for (int i = 0; i < mat.rows; ++i) {
            std::memcpy(&img_frame.at(i, 0, 0), mat.ptr<unsigned char>(i),
                        mat.cols * mat.channels());
        }
    }
    ```

- <b class="tab-title">Numpy (Python)</b>
    ImageFrame → ndarray
    ```python
    img_np = img_frame.numpy()
    assert img_np.shape == (height, width, img_frame.channels())
    ```
    ndarray → ImageFrame
    ```python
    # 例: uint8, BGR形式の場合
    assert img_np.dtype == np.uint8
    assert img_np.shape == (height, width, 3)
    img_frame = ImageFrame.from_numpy(img_np, ImageColorMode.BGR)
    ```

</div>

<details><summary>C++ OpenCVと相互変換できるImageFrame (〜ver1.11まで)</summary>

画像データを表す型として webcface::ImageFrame があります。
cv::Mat形式 (`CV_8UC1`,`CV_8UC3`,`CV_8UC4` フォーマットのみ) の画像データをImageFrameのコンストラクタに渡すことができます。
```cpp
cv::Mat data;
webcface::ImageFrame frame(data, webcface::ImageColorMode::rgb);
```
ImageColorModeとしては `gray`(8bitグレースケール)、`rgb`(8bit×3)、`bgr`(8bit×3)、`rgba`(8bit×4)、`bgra`(8bit×4) から元の画像データのフォーマットを指定してください。
\note
ImageFrameオブジェクト内部では画像データは`shared_ptr<vector<unsigned char>>`で保持されます。  
ImageFrameのコンストラクタでcv::Mat内部の画像データがすべてコピーされます。  
ImageFrameをコピーした場合は画像データが共有されます(コピーされません)。

ImageFrameをImageにsetして送信します。
```cpp
wcli.image("hoge").set(frame);
```
 (C++のみ) set() の代わりに代入演算子(Image::operator=)でも同様のことができます。
```cpp
wcli.image("hoge") = frame;
```

ImageFrame::mat() でcv::Mat形式に変換できますが、その場合画像データ本体はImageFrameが保持しているためcv::Mat画像を使う間ImageFrameオブジェクトが破棄されないようにしてください。
(例えば `wcli.member("foo").image("hoge").get().mat()` はできません)  
また、Imageを一度変数に入れて使うと、 Image::mat() で直接cv::Matに変換することもできます (この場合データ本体はImageが保持しています)
```cpp
webcface::Image image = wcli.member("foo").image("hoge");
while(true){
    cv::Mat data = image.mat(); // 内部で get() → mat() が呼ばれる
    // ...
}
```
\note
圧縮された画像が入ったImageFrameをcv::Matに変換すると画像をデコードして返します。

</details>

## 受信

<div class="tabbed">

- <b class="tab-title">C++</b>
    Member::image() でImageクラスのオブジェクトが得られ、
    Image::request() で画像のリクエストをした後
    Image::tryGet() で受信した画像を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。
    ```cpp
    wcli.member("foo").image("hoge").request(); // 何も指定しない→元画像をそのまま受信
    while(true){
        std::optional<webcface::ImageFrame> frame = wcli.member("foo").image("hoge").tryGet();
        if(frame){
            //...
        }
    }
    ```
    * リクエストした画像を受信するまでの間tryGet()はstd::nulloptを返します。
    * get() を使うとstd::nulloptの代わりに空のImageFrameを返します。
        * (ImageFrame::empty() で空かどうかを判別できます)
    * ImageをImageFrameにキャストすることでも get() と同様に画像が得られます。  
    * request() を1度も呼ばずに tryGet() や get() した場合、デフォルトのオプションで(元画像のフォーマットで)リクエストされます。
    * ImageFrameの中のデータには
    dataPtr() (`shared_ptr<vector<unsigned char>>`型)、
    data() (`vector<unsigned char>&` 型)、
    at(y, x, ch) (`unsigned char &`型)
    でアクセスできます。

    request() の引数に画像のサイズと色モードを指定すると、サーバー側で指定したフォーマットに変換されたものが取得できます。  
    また送信側が高頻度で画像を更新する場合、フレームレートを指定するとそれより遅い頻度で受信させることもできます。
    ```cpp
    // 64x48にリサイズ、さらに要素をRGBの順にし、1秒に5枚まで受信する
    wcli.member("foo").image("hoge").request(sizeWH(64, 48),
                                             webcface::ImageColorMode::rgb, 5);
    // 幅が64になるようリサイズ(縦横比を維持)
    wcli.member("foo").image("hoge").request(sizeWH(64, std::nullopt),
                                             webcface::ImageColorMode::rgb);
    // サイズは元画像のままグレースケール
    wcli.member("foo").image("hoge").request(std::nullopt, webcface::ImageColorMode::gray);
    ```
    
    また jpeg, png, webp に圧縮した画像をリクエストすることでも通信量を減らすことができます。
    詳細は webcface::Image::request() を参照
    ```cpp
    // 64x48にリサイズ、さらにjpeg圧縮、1秒に5枚まで受信する
    wcli.member("foo").image("hoge").request(sizeWH(64, 48),
                                             webcface::ImageCompressMode::jpeg, 50,
                                             5);
    ```

    圧縮された画像の場合 dataPtr() または data() を使ってバイナリデータとして取得できますが、 at() での要素アクセスはできません。

- <b class="tab-title">JavaScript</b>
    Member.image() でImageクラスのオブジェクトが得られ、
    Image.request() で画像のリクエストをした後
    Image.tryGet() で受信した画像を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```ts
    wcli.member("foo").image("hoge").request({}); // 何も指定しない→元画像をそのまま受信
    while(true){
        const frame: ImageFrame | null = wcli.member("foo").image("hoge").tryGet();
        if(frame){
            //...
        }
    }
    ```
    * リクエストした画像を受信するまでの間tryGet()はnullを返します。
    * Image.get() を使うとnullの代わりに空のImageFrameを返します。
    * request() を1度も呼ばずに tryGet() や get() した場合、デフォルトのオプションで(元画像のフォーマットで)リクエストされます。
    * 受信した画像は ImageFrame.data から取得できます。

    request() の引数に画像のサイズと色モードを指定すると、サーバー側で指定したフォーマットに変換されたものが取得できます。  
    また送信側が高頻度で画像を更新する場合、フレームレートを指定するとそれより遅い頻度で受信させることもできます。  
    request() の引数については [ImageReq](https://na-trium-144.github.io/webcface-js/interfaces/ImageReq.html) を参照
    ```ts
    // 64x48にリサイズ、さらに要素をRGBの順にし、1秒に5枚まで受信する
    wcli.member("foo").image("hoge").request({
        width: 64,
        height: 48,
        colorMode: imageColorMode.rgb,
        frameRate: 5,
    });
    // 幅が64になるようリサイズ(縦横比を維持)
    wcli.member("foo").image("hoge").request({
        width: 64,
        colorMode: imageColorMode.rgb,
    });
    // サイズは元画像のままグレースケール
    wcli.member("foo").image("hoge").request({
        colorMode: imageColorMode.gray,
    });
    ```
    
    また jpeg, png, webp に圧縮した画像をリクエストすることでも通信量を減らすことができます。
    ```ts
    // 64x48にリサイズ、さらにjpeg圧縮、1秒に5枚まで受信する
    wcli.member("foo").image("hoge").request({
        width: 64,
        height: 48,
        compressMode: imageCompressMode.jpeg,
        quality: 50,
        frameRate: 5,
    });
    ```

    圧縮された画像の場合も ImageFrame.data からバイナリデータとして取得できます。  
    また、 ImageFrame.toBase64() でbase64にエンコードすることができます。

- <b class="tab-title">Python</b>
    Member.image() でImageクラスのオブジェクトが得られ、
    Image.request() で画像のリクエストをした後
    Image.try_get() で受信した画像を取得できます。

    例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

    ```python
    wcli.member("foo").image("hoge").request() # 何も指定しない→元画像をそのまま受信
    while True:
        frame = wcli.member("foo").image("hoge").try_get()
        if frame is not None:
            # ...
    ```
    * リクエストした画像を受信するまでの間try_get()はNoneを返します。
    * Image.get() を使うとNoneの代わりに空のImageFrameを返します。
        * (ImageFrame.empty() で空かどうかを判別できます)
    * request() を1度も呼ばずに try_get() や get() した場合、デフォルトのオプションで(元画像のフォーマットで)リクエストされます。
    * 受信した画像は ImageFrame.data, ImageFrame.numpy() から取得できます。

    request() の引数に画像のサイズと色モードを指定すると、サーバー側で指定したフォーマットに変換されたものが取得できます。  
    また送信側が高頻度で画像を更新する場合、フレームレートを指定するとそれより遅い頻度で受信させることもできます。  
    ```python
    # 64x48にリサイズ、さらに要素をRGBの順にし、1秒に5枚まで受信する
    wcli.member("foo").image("hoge").request(
        width=64,
        height=48,
        color_mode=ImageColorMode.RGB,
        frame_rate=5,
    )
    # 幅が64になるようリサイズ(縦横比を維持)
    wcli.member("foo").image("hoge").request(
        width=64,
        color_mode=ImageColorMode.RGB,
    )
    # サイズは元画像のままグレースケール
    wcli.member("foo").image("hoge").request(
        color_mode=ImageColorMode.GRAY,
    )
    ```
    
    また jpeg, png, webp に圧縮した画像をリクエストすることでも通信量を減らすことができます。
    ```python
    # 64x48にリサイズ、さらにjpeg圧縮、1秒に5枚まで受信する
    wcli.member("foo").image("hoge").request(
        width=64,
        height=48,
        compress_mode=ImageCompressMode.JPEG,
        quality=50,
        frame_rate=5,
    )
    ```

    圧縮された画像の場合も ImageFrame.data からバイナリデータとして取得できますが、
    ImageFrame.numpy() は使えません。

</div>


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [6-1. Canvas2D](61_canvas2d.md) | [6-3. Canvas3D](63_canvas3d.md) |

</div>
