# バックエンド

## C++

* ヘッダーは inc/webcface/ 以下にあり、`#include <webcface/...hpp>`でインクルードできます。
	* `<webcface/webcface.hpp>`をincludeするとexternal/以下のファイル以外すべてincludeされます。
* 外部ライブラリに依存する機能は inc/webcface/external/ 以下にあります
	* webcface側ではこれらの外部ライブラリにリンクすることはないので、各プロジェクトで使用しているライブラリをリンクしてください
* namespaceは`WebCFace`です
	* `WebCFace`の中でさらにいくつかのnamespaceに分かれていますが、基本的にすべてinline namespaceなので使用時は`WebCFace::`だけでもokです

## python

* `import webcface`でpythonからwebcfaceが使用できます
* 関数名は基本的にC++の`WebCFace::startServer` → pythonの`webcface.start_server`のように、大文字を小文字にしアンダーバーを加えたものになっています
* クラス名はC++の`WebCFace::PyButton` → pythonの`webcface.Button`のように大文字はそのまま、頭にPyがつくものはPyを取ったものです

