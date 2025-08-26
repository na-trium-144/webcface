# usage: fetch_webui.py webui_version dest_dir

import sys
import os
import urllib.request
import tarfile
import shutil

_, webui_ver, dest_dir = sys.argv
url = f"https://github.com/na-trium-144/webcface-webui/releases/download/v{webui_ver}/webcface-webui_{webui_ver}.tar.gz"
output_tar = f"webcface-webui_{webui_ver}.tar.gz"

os.chdir(dest_dir)
if os.path.exists(output_tar) and os.path.exists("dist"):
    print("WebUI is already downloaded.")
    sys.exit(0)

urllib.request.urlretrieve(url, output_tar)

if os.path.exists("dist"):
    shutil.rmtree("dist")

with tarfile.open(output_tar, 'r:gz') as tar:
    # Extract all contents to the current directory
    tar.extractall()
