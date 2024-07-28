from magick_msvc_config_common import *

# 出力は 1行目: include dir (,区切り) or 空文字列
#        2行目: lib dir or 空文字列
#        3行目以降: ライブラリファイル名

import os
import re
print(",".join([
    os.path.relpath(os.path.join(im_win_dir, "ImageMagick", "Magick++", "lib"), source_dir),
    os.path.relpath(os.path.join(im_win_dir, "ImageMagick"), source_dir),
]))

libs = []
core_re = re.compile(r'^Project\([^\)]*\) = "CORE_([^"]*)",.*')
with open(im_sln, "r") as f:
    for line in f.readlines():
        m = core_re.match(line)
        if m:
            libs.append(f"CORE_{libtype}_{m.groups()[0]}_.lib")
if libs:
    print(os.path.join(im_win_dir, "Output", "lib"))
    for lib in libs:
        print(lib)
else:
    print()
