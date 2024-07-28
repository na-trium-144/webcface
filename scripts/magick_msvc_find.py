# 引数にmagickのパスと "RL" or "DB"
# 出力は 1行目: include dir or 空文字列
#        2行目: lib dir or 空文字列
#        3行目以降: ライブラリファイル名

import sys
import os
import re

_, magick_path, libtype = sys.argv

if os.path.exists(os.path.join(magick_path, "include", "Magick++.h")):
    print(os.path.join(magick_path, "include"))
else:
    print()

libs = []
lib_re = re.compile(fr"^[A-Z]+_{libtype}_.*\.lib$")
for lib in os.listdir(os.path.join(magick_path, "lib")):
    if lib_re.match(lib):
        libs.append(lib)
if libs:
    print(os.path.join(magick_path, "lib"))
    for lib in libs:
        print(lib)
else:
    print()
