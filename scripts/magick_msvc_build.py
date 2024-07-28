from magick_msvc_config_common import *
import os
import shutil

config = {
    "DB": "Debug",
    "RL": "Release",
}[libtype]

run("msbuild", im_sln,
    "/m",
    f"/p:PlatformToolset={vs_toolchain},Configuration={config},Platform={arch}",
)
print("copying libraries to build_dir")
for lib in os.listdir(os.path.join(im_win_dir, "Output", "lib")):
    if lib.startswith("CORE_" + libtype):
        # print(lib)
        shutil.copy(os.path.join(im_win_dir, "Output", "lib", lib), build_dir)
