# インストール

## 環境構築
C++20以上が使えるコンパイラ、Python 3.7以上、node.js 16?(要検証)以上 が必要です

## TL;DR
普通はここまでしなくてもいいが、確実な方法
```bash
sudo apt install libjsoncpp-dev uuid-dev openssl libssl-dev zlib1g-dev
sudo apt install libcairo2-dev libpango1.0-dev libjpeg-dev libgif-dev librsvg2-dev
git submodule update --init --recursive
cd frontend
npm install
cd ..
sudo pip install -I .
```

## Drogon
* Drogonのインストールは任意です
* インストールされていない場合、submoduleに入っているdrogonをコンパイルして使います
    * ただしインストール時と同じ依存ライブラリ(以下)が必要になります
```bash
sudo apt install libjsoncpp-dev uuid-dev openssl libssl-dev zlib1g-dev
```
* インストールされている場合はそれを使用します
    * インストール方法は https://github.com/drogonframework/drogon/wiki/ENG-02-Installation を参照
    * ただしcmake時に`-DBUILD_SHARED_LIBS=on`にする必要があります

## WebCFace

### (推奨)sudo pipでインストールする

このリポジトリをどこかにcloneし、`git submodule update`をしたら、
```sh
sudo pip install -I .
```
でWebCFaceが /usr/local 以下にインストールされます

`-I`をつけると前のバージョンのwebcfaceを削除しません

pipでインストールするだけでpythonからもcmakeからも使えるようになります

pythonから使う場合は
```py
import webcface
```

cmakeから使う場合は
```cmake
find_package(webcface)
```
を追加し、また、target_link_librariesに`webcface::webcface`を追加してください

pythonからしか使わない場合、またはpythonの仮想環境で使いたい場合はsudoをしないpipでインストールしてもよいです。


### make installでインストールする

pipを使わない場合の手順は以下

```sh
mkdir build
cd build
cmake ..
make
sudo make install
```
でWebCFaceが /usr/local 以下にインストールされます。

(makeではなくninjaでも問題ない)

### WebCFaceをsubmoduleで追加する

C++から利用する場合で、インストールしたくない場合はこちら

このリポジトリをsubmoduleで追加し、`git submodule update --init --recursive`をします

CMakeLists.txtに
```cmake
set(WEBCFACE_LEGACY false)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/webcface)
```
のようにwebcfaceのディレクトリを追加し、また、target_link_librariesに`webcface::webcface`を追加してください

### 旧仕様

旧バージョンとの互換性のために残している機能です。推奨しません。

このリポジトリをsubmoduleで追加し、`git submodule update --init --recursive`をします

CMakeLists.txtに
```cmake
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/webcface)
```
のようにwebcfaceのディレクトリを追加し、また、target_link_librariesに`webcface`を追加してください

機体pcにrsyncして動かす場合など、実行時のパスがビルド時のパスと異なる場合はadd_subdirectoryの前に
```cmake
set(WEB_ROOT_DIR webcfaceディレクトリの絶対パス)
```
を追加してください

## トラブルシューティング
### npm installがConnectionResetなどでエラーになる
ラズパイなどよわよわな環境ではC++のビルド中にnpm installをすると重すぎてタイムアウトしてしまう

その場合は事前にnpm installをしておくとよい
```bash
cd frontend
npm install
```

### node-canvasのインストールでエラー(404 not found)になる
x86_64の場合はnpmでインストールできるので不要だが、armの場合ソースからコンパイルすることになるので以下が必要になる
(コンパイルはnpm install中に自動で行われる)

```bash
sudo apt install libcairo2-dev libpango1.0-dev libjpeg-dev libgif-dev librsvg2-dev
```
ubuntu以外の場合 https://github.com/Automattic/node-canvas を参照
