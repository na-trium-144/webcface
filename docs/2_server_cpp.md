# server (c++)

WebCFaceのサーバープログラムです。WebCFaceを動かすのに必須になります。

C++20以上が使えるコンパイラが必要です

## 依存ライブラリ

```bash
sudo apt install libjsoncpp-dev uuid-dev zlib1g-dev
```

## ビルド

```bash
git submodule update --init --recursive
mkdir build
cd build
cmake ..
make
# sudo make install
```

installできるようにする予定ですが現在は未実装です。

## 起動

```bash
./webcface-server
```
