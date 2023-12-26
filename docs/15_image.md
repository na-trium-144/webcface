# Image

![c++ ver1.3](https://img.shields.io/badge/1.3~-00599c?logo=C%2B%2B)
![js ver1.2](https://img.shields.io/badge/1.2~-f7df1e?logo=JavaScript&logoColor=black)

API Reference �� webcface::Image

�摜�f�[�^�𑗎�M���܂��B

Member::image() ��Image�N���X�̃I�u�W�F�N�g�������܂�
```cpp
webcface::Image image_hoge = wcli.member("a").image("hoge");
```

Member::images() �ł���Member�����M���Ă���image�̃��X�g�������܂�
```cpp
for(const webcface::Image &v: wcli.member("a").images()){
	// ...
}
```

Member::onImageEntry() �ŐV�����f�[�^���ǉ����ꂽ�Ƃ��̃R�[���o�b�N��ݒ�ł��܂�
```cpp
wcli.member("a").onImageEntry().appendListener([](webcface::Image v){ /* ... */ });
```

TODO: �h�L�������g������


<div class="section_buttons">

| Previous |     Next |
|:---------|---------:|
| [Value](13_view.md) | [Func](30_func.md) |

</div>
