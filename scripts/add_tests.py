# GoogleTestの `TEST()`, `TEST_F()`, `TEST_P()` を使うC++ソースファイルを引数に渡すと、
# それをパースして、含まれるテスト名をstdoutに出力

import sys
import re

test_re = re.compile(r"^TEST_?[FP]?\( *([^, ]*) *, *([^\)]*)\)")

for file in sys.argv[1:]:
    with open(file, 'r', encoding='utf-8') as f:
        for line in f.readlines():
            m = test_re.match(line)
            if m:
                print(".".join(m.groups()))
