import glob
import os

base_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")

for s_dir in ["client", "common"]:
    root_dir = os.path.join(s_dir, "include")
    os.chdir(os.path.join(base_dir, root_dir))
    for h in glob.glob("**/*.h", recursive=True):
        if "internal" not in h:
            print(";".join([
                root_dir,
                os.path.dirname(h),
                os.path.basename(h)
            ]))
