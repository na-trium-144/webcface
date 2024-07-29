# 引数に sourcedir, builddir, "RL" or "DB", cl_version, cpu

im_win_tag = '6a18e9e966620b574bfd4529ea1502c0d655814d'
im_tag = '841f033f0'

import sys
import os
import subprocess

_, source_dir, build_dir, libtype, cl_ver, cpu = sys.argv

if cl_ver >= "19.30":
    vs_toolchain = "v143"
    vs = 2022
elif cl_ver >= "19.20":
    vs_toolchain = "v142"
    vs = 2019
elif cl_ver >= "19.10":
    vs_toolchain = "v141"
    vs = 2017
else:
    print(f"Compiler version {cl_ver} is older than vs2017(19.10)")
    sys.exit(1)

config = {
    "DB": "Debug",
    "RL": "Release",
}[libtype]

# mesonのcpu_family -> vsのarch
if cpu == "x86":
    im_arch = "x86"
elif cpu == "x86_64":
    im_arch = "x64"
elif cpu == "aarch64":
    im_arch = "arm64"
else:
    print(f"CPU family {cpu} is not supported")
    sys.exit(1)

im_win_dir = os.path.join(build_dir, "imagemagick-windows")
im_sln = os.path.join(im_win_dir, f"IM7.StaticDLL.{im_arch}.sln")
im_configure_dir = os.path.join(im_win_dir, "Configure")
im_configure = os.path.join(im_configure_dir, "Configure.exe")

def is_old(file):
    if not os.path.exists(file):
        print(f"{file} does not exist")
        return True
    if os.path.getmtime(file) < os.path.getmtime(os.path.join(source_dir, "meson.build")):
        print(f"{file} is older than meson.build")
        return True
    if os.path.getmtime(file) < os.path.getmtime(os.path.abspath(__file__)):
        print(f"{file} is older than {__file__}")
        return True
    print(f"{file} is up to date")
    return False

def run(*cmd):
    print("running " + " ".join(f"'{c}'" for c in cmd) + "...")
    subprocess.run(cmd,
        check=True,
        stderr=subprocess.STDOUT,
    )
