import os
import glob

prefix = os.environ["MESON_INSTALL_PREFIX"]
if "DESTDIR" in os.environ and os.environ["DESTDIR"]:
    if prefix.startswith("/"):
        prefix = prefix.lstrip("/")
    prefix = os.path.join(os.environ["DESTDIR"], prefix)

pc = glob.glob(os.path.join(prefix, "**", "webcface.pc"), recursive=True)
with open(pc[0], "r") as f:
    pc_data = f.readlines()
with open(pc[0], "w") as f:
    for line in pc_data:
        if not line.startswith("Requires.private"):
            f.write(line)
