from magick_msvc_config_common import *
import os

if not os.path.exists(im_win_dir):
    run("git", "clone",
        "--single-branch", "-b", "main",
        # "-b", im_win_tag,
        "https://github.com/ImageMagick/ImageMagick-Windows.git",
        im_win_dir,
    )
# else:
os.chdir(im_win_dir)
run("git", "fetch", "origin", "main")
run("git", "checkout", im_win_tag)

if is_old(im_sln):
    if is_old(im_configure):
        os.chdir(im_win_dir)
        run("CloneRepositories.IM7.cmd", im_tag)
        # patch
        with open(os.path.join(im_win_dir, "Projects", "MagickCore", "magick-baseconfig.h.in"), "a") as f:
            f.write("\n#pragma warning(disable: 4201)\n")
        
        os.chdir(im_configure_dir)
        run("devenv", "/upgrade", "Configure.2017.sln")
        run("msbuild", "Configure.2017.sln",
            "/m",
            f"/p:PlatformToolset={vs_toolchain}",
        )
        if not os.path.exists(im_configure):
            print("Failed to build configure.exe")
            sys.exit(1)

    os.chdir(im_configure_dir)
    run("Configure.exe",
        "/noWizard", "/noAliases", "/noDpc", "/noHdri", "/noOpenMP", "/Q8",
        f"/{arch}", f"/VS{vs}", "/smtd",
    )
    if not os.path.exists(im_sln):
        print("Failed to configure imagemagick-windows")
        sys.exit(1)

    # でっちあげでdllexportを消す
    with open(os.path.join(im_win_dir, "Dependencies", "pango", "pango", "config.h"), "a") as f:
        f.write("\n#define _PANGO_EXTERN\n")

