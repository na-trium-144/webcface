import glob
import os
import subprocess
import sys
from typing import List
import time

# 引数に並列実行数
if len(sys.argv) >= 2:
    j_num = int(sys.argv[1])
else:
    j_num = 1

base_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
jobs: List[subprocess.Popen] = []
fail = False

for s_dir in os.listdir(base_dir):
    if (
        s_dir.startswith("build")
        or s_dir.startswith(".")
        or s_dir == "subprojects"
        or not os.path.isdir(os.path.join(base_dir, s_dir))
    ):
        continue
    os.chdir(os.path.join(base_dir, s_dir))
    for src in glob.glob("**/*.cc", recursive=True):
        print(os.path.join(s_dir, src))
        jobs.append(
            subprocess.Popen(
                [
                    "clang-tidy",
                    "-p",
                    os.path.join(base_dir, "build"),
                    src,
                    "--extra-arg=-Wno-unknown-warning-option",
                ],
            ),
        )

        while len(jobs) >= j_num or (fail and len(jobs) > 0):
            for j in jobs[:]:
                if j.poll() is not None:
                    if j.returncode != 0:
                        fail = True
                    jobs.remove(j)
            time.sleep(1)
        if fail:
            sys.exit(1)
