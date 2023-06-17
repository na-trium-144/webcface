# ログを送る

see also WebCFace::Logger

c++の場合`<webcface/logger.hpp>`

* ブラウザの「ログ表示」をクリックするとログが表示されます

## std::cout, std::cerr を自動的に送る

* c++ → WebCFace::initStdLogger
* python → webcface.init_std_logger()
	* python版init_std_loggerで送られるのはstd::coutとstd::cerrで、sys.stdoutとsys.stderrではありません(python側でprintなどで表示する値は送られません)
* これをプログラムの最初で呼び出すと以降の出力が自動で転送されます
	* これを呼び出した以降の出力のみが転送されるので、mainの最初とかがよい
* 重要度の設定は現状ではすべて0になります(issue #58)

## sys.stdout, sys.stderr を自動的に送る

* python → webcface.init_sys_logger()
* これはpybindではなくpythonのラッパーで実装されています、 webcface/logger.py を参照
