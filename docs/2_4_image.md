# 画像を送る

* フロントエンドでは「カメラ画像」ページで見ることができます

## base64
* c++ → `<webcface/registration.hpp>`の WebCFace::addImage
* base64でエンコードした文字列を送ることで画像として表示できます
* 実際にはOpenCVなどの画像ライブラリと組み合わせて使うと思いますが、後述のように外部ライブラリと連携をする機能があるので、このbase64を送る関数を直接使うことはないと思います

## OpenCV

```cpp
//これはwebcface.hppには含まれない
#include <webcface/external/opencv.hpp>
cv::Mat img;
WebCFace::addImage("image1", img);
```
でimgに入っている画像が次のsendData時に送られます
* imgが変更されるたびにaddImageする必要があります


