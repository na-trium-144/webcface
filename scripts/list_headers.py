import glob
import os

os.chdir(os.path.join(os.path.dirname(__file__), ".."))

for s_dir in ["client", "encoding"]:
    root_dir = os.path.join(s_dir, "include")
    for h in glob.iglob(
        "**/*.h",
        root_dir=root_dir,
        recursive=True,
    ):
        if "internal" not in h:
            print(";".join([
                root_dir,
                os.path.dirname(h),
                os.path.basename(h)
            ]))
