# client (c++)

C++でクライアントを使う場合のドキュメントです。

## CMake

(いずれinstallしてfind_packageできるようにする予定)

このプロジェクトをsubmoduleで追加して
```cmake
add_subdirectory(path/to/webcface)
target_link_libraries(target PRIVATE webcface)
```

## Client

```cpp
#include <webcface/webcface.h>

WebCFace::Client wc("client_name");
```

WebCFace::Client オブジェクトを作ればこれを通してサーバーと通信ができます。

### Value

double型の値を送受信できます。

api → WebCFace::Value

```cpp
wc.value("value_name") = 123;
wc.send(); // セットされた値をまとめて送信

std::optional<double> v1 = wc.value("another_client", "value_name").try_get(); // 受信
double v2 = wc.value("another_client", "value_name"); // 直接キャスト
```

### Text

std::stringの値を送受信できます。
使い方は Value とほぼ同じです。

api → WebCFace::Text

### Func

関数を登録し、他クライアントから呼び出されたときに実行できます。

```cpp
wc.func("func_name") = func;

std::future<FuncResult> fr = wc.func("another_client", "func").run();
```

api → WebCFace::Func

