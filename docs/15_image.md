# Image

![c++ ver1.3](https://img.shields.io/badge/1.3~-00599c?logo=C%2B%2B)
![js ver1.2](https://img.shields.io/badge/1.2~-f7df1e?logo=JavaScript&logoColor=black)

API Reference → webcface::Image

画像データを送受信します。

Member::image() でImageクラスのオブジェクトが得られます
```cpp
webcface::Image image_hoge = wcli.member("a").image("hoge");
```

Member::images() でそのMemberが送信しているimageのリストが得られます
```cpp
for(const webcface::Image &v: wcli.member("a").images()){
	// ...
}
```

Member::onImageEntry() で新しくデータが追加されたときのコールバックを設定できます
```cpp
wcli.member("a").onImageEntry().appendListener([](webcface::Image v){ /* ... */ });
```

TODO: ドキュメントを書く


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Value](13_view.md) | [Func](30_func.md) |

</div>
