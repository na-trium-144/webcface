# Image

\since
<span class="since-c">1.3</span>
<span class="since-js">1.2</span>
\sa
* C++ webcface::Image
* JavaScript [Image](https://na-trium-144.github.io/webcface-js/classes/Image.html)
* Python 未実装 <!--[webcface.Image](https://na-trium-144.github.io/webcface-python/webcface.image.html#webcface.image.Image)-->

画像データを送受信します。

## コマンドライン

```sh
webcface-cv-capture 0
```
などとするとOpenCVのVideoCaptureでキャプチャできるWebカメラなどの画像をwebcfaceに送信することができます。

## 送信

Client::image からImageオブジェクトを作り、 Image::set() で画像データを代入し、Client::sync()することで送信されます

<div class="tabbed">

- <b class="tab-title">C++ (with OpenCV)</b>
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
- <b class="tab-title">C++</b>
    画像データを表す型として webcface::ImageFrame があります。
    生の画像データがあればImageFrameのコンストラクタに渡すことができます。
    ```cpp
    unsigned char data[640 * 480 * 3]; /* = {...}; */
    webcface::ImageFrame frame(480, 640, data, webcface::ImageColorMode::rgb); // 画像サイズは 縦, 横 で指定
    ```
    ImageColorModeとしては `gray`(8bitグレースケール)、`rgb`(8bit×3)、`bgr`(8bit×3)、`rgba`(8bit×4)、`bgra`(8bit×4) が扱えます。
    \note
    ImageFrameオブジェクト内部では画像データは`shared_ptr<vector<unsigned char>>`で保持されます。  
    ImageFrameのコンストラクタでdataの内容がすべてコピーされます。  
    ImageFrameをコピーした場合は画像データが共有されます(コピーされません)。
    \todo
    C++とJavaScriptでImageFrameのコンストラクタのサイズ指定が違うので、そのうち仕様変更をして統一する必要がある

    ImageFrameをImageにsetして送信します。
    ```cpp
    wcli.image("hoge").set(frame);
    ```
     (C++のみ) set() の代わりに代入演算子(Image::operator=)でも同様のことができます。
    ```cpp
    wcli.image("hoge") = frame;
    ```

- <b class="tab-title">JavaScript</b>
    画像データを表す型として [ImageFrame](https://na-trium-144.github.io/webcface-js/classes/ImageFrame.html) があります。
    生の画像データがあればImageFrameのコンストラクタに渡すことができます。
    ```ts
    import { ImageFrame, imageColorMode } from "webcface";
    const data: ArrayBuffer = /* ... */;
    const frame: ImageFrame = new ImageFrame(640, 480, data, imageColorMode.rgb); // 画像サイズは 横, 縦 で指定
    ```
    imageColorModeとしては `gray`(8bitグレースケール)、`rgb`(8bit×3)、`bgr`(8bit✕3)、`rgba`(8bit✕4)、`bgra`(8bit✕4) が扱えます。
    \todo
    C++とJavaScriptでImageFrameのコンストラクタのサイズ指定が違うので、そのうち仕様変更をして統一する必要がある

    ImageFrameをImageにsetして送信します。
    ```cpp
    wcli.image("hoge").set(frame);
    ```

</div>

## 受信

Member::image() でImageクラスのオブジェクトが得られ、
Image::request() で画像のリクエストをした後
Image::tryGet() で受信した画像を取得できます。

リクエスト時の引数で画像のサイズとフォーマットを指定でき、その場合サーバー側で変換してから送られます。  
また jpeg, png, webp に圧縮した画像をリクエストしたり、フレームレートを指定して画像を受信する頻度を下げることで、
Wi-Fi経由で受信する場合に通信量を減らすことができます。  
詳細は webcface::Image::request() を参照

例えば`foo`というクライアントの`hoge`という名前のデータを取得したい場合は次のようにします。

<div class="tabbed">

- <b class="tab-title">C++ (with OpenCV)</b>
    ```cpp
    wcli.member("foo").image("hoge").request(); // 何も指定しない→元画像をそのまま受信
    while(true){
        std::optional<webcface::ImageFrame> frame = wcli.member("foo").image("hoge").tryGet();
        if(frame){
            cv::Mat data = frame.mat();
            // ...
        }
    }
    ```
    Image::get() はまだ受信できていない場合std::nulloptの代わりに空のImageFrameを返します。
    (ImageFrame::empty() で空かどうかを判別できます)  
    ImageをImageFrameにキャストすることでも get() と同様に画像が得られます。  
    request() を呼ばずに tryGet() や get() した場合、デフォルトのオプションで(元画像のフォーマットで)リクエストされます。

    受信した画像は cv::Mat に変換して使うことができます。  
    圧縮された画像の場合は ImageFrame::data() からバイナリデータを取得できます。

    \warning
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

- <b class="tab-title">C++</b>
    ```cpp
    wcli.member("foo").image("hoge").request(); // 何も指定しない→元画像をそのまま受信
    while(true){
        std::optional<webcface::ImageFrame> frame = wcli.member("foo").image("hoge").tryGet();
        if(frame){
            //...
        }
    }
    ```
    Image::get() はまだ受信できていない場合std::nulloptの代わりに空のImageFrameを返します。
    (ImageFrame::empty() で空かどうかを判別できます)  
    ImageをImageFrameにキャストすることでも get() と同様に画像が得られます。  
    request() を呼ばずに tryGet() や get() した場合、デフォルトのオプションで(元画像のフォーマットで)リクエストされます。

    受信した画像は ImageFrame::data() から取得できます。また、 `ImageFrame::at(y, x, channel)` で要素にアクセスすることもできます。  
    圧縮された画像の場合 ImageFrame::data() からバイナリデータとして取得できますが、`at()`での要素アクセスはできません。

- <b class="tab-title">JavaScript</b>
    requestのオプションは [ImageReq](https://na-trium-144.github.io/webcface-js/interfaces/ImageReq.html) で指定します。
    ```ts
    wcli.member("foo").image("hoge").request({}); // 何も指定しない→元画像をそのまま受信
    while(true){
        const frame: ImageFrame | null = wcli.member("foo").image("hoge").tryGet();
        if(frame){
            //...
        }
    }
    ```
    Image::get() はまだ受信できていない場合nullの代わりに空のImageFrameを返します。  
    request() を呼ばずに tryGet() や get() した場合、デフォルトのオプションで(元画像のフォーマットで)リクエストされます。

    受信した画像は ImageFrame.data から取得できます。
    圧縮された画像の場合も ImageFrame.data からバイナリデータとして取得できます。  
    また、 ImageFrame.toBase64() でbase64にエンコードすることができます。

</div>

### 時刻

Image::time() でその値が送信されたとき(そのMemberがsync()したとき)の時刻が得られます。

<!-- \note Pythonでは Member.sync_time() -->

### Entry

~~Member::images() で~~ そのMemberが送信しているimageのリストが得られます  
<span class="since-c">1.6</span>
Member::imageEntries() に変更

また、Member::onImageEntry() で新しくデータが追加されたときのコールバックを設定できます

いずれも使い方は [Value](./10_value.md) と同様なのでそちらを参照してください

### Event

受信したデータが変化したときにコールバックを呼び出すことができます。
コールバックを設定することでもその値はリクエストされます。

また、データが変化したどうかに関わらずそのMemberがsync()したときにコールバックを呼び出したい場合は Member::onSync() が使えます

使い方は [Value](./10_value.md) と同様なのでそちらを参照してください


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Canvas2D](14_canvas2d.md) | [Canvas3D](20_canvas3d.md) |

</div>
