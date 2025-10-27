#!/bin/sh
set -e
INSTALL_DIR="/opt/webcface"
APP_INSTALL_DIR="/Applications"
OPT_DIR=
DEFAULT_VERSION=3.2.0
AVAILABLE_VERSIONS="3.2.0 3.1.1-1 3.1.1 2.9.0-1 2.9.0 2.8.0 2.7.0 2.5.2-1 2.5.2 2.5.1 2.5.0-2 2.5.0-1 2.5.0 2.4.2 2.4.1-2 2.4.1-1 2.4.1 2.4.0-1 2.4.0 2.3.0 2.2.1 2.2.0 2.1.0 2.0.5 2.0.4"

usage(){
    echo "Usage: installer.sh [-a|-x] [-y] [-d DIR] [-u] [VERSION]"
    echo "  -a: Install with apt-get (only for Debian-based systems)"
    echo "  -x: Extract archive manually, without using apt-get"
    echo "  -d DIR: Extract archive to DIR (default: /opt/webcface)"
    echo "  -u: Extract webcface-desktop.app to \$HOME/Applications, instead of the default /Applications (only for macOS)"
    echo "  -y: Assume yes for all prompts"
    echo "  VERSION: Version to install, without 'v' prefix (default: $DEFAULT_VERSION)"
    echo "           Available versions: $AVAILABLE_VERSIONS"
    exit 1
}

while getopts axd:uy OPT; do
    case $OPT in
    a) OPT_APT=1 ;;
    x) OPT_APT=0 ;;
    d) OPT_DIR=1; INSTALL_DIR="$OPTARG" ;;
    u) OPT_USER=1 ;;
    y) OPT_YES=1 ;;
    *) echo; usage ;;
    esac
done
shift $((OPTIND - 1))

if [ -n "$1" ]; then
    VERSION=$1
    shift 1
else
    VERSION=$DEFAULT_VERSION
fi
case $VERSION in
3.2.0)   WEBCFACE_VERSION=3.2.0; WEBUI_VERSION=1.15.0; TOOLS_VERSION=3.0.1-1;;
3.1.1-1) WEBCFACE_VERSION=3.1.1; WEBUI_VERSION=1.15.0; TOOLS_VERSION=3.0.1;;
3.1.1)   WEBCFACE_VERSION=3.1.1; WEBUI_VERSION=1.15.0; TOOLS_VERSION=3.0.0;;
2.9.0-1) WEBCFACE_VERSION=2.9.0; WEBUI_VERSION=1.14.0; TOOLS_VERSION=2.3.1;;
2.9.0)   WEBCFACE_VERSION=2.9.0; WEBUI_VERSION=1.14.0; TOOLS_VERSION=2.3.0;;
2.8.0)   WEBCFACE_VERSION=2.8.0; WEBUI_VERSION=1.13.0; TOOLS_VERSION=2.2.0;;
2.7.0)   WEBCFACE_VERSION=2.7.0; WEBUI_VERSION=1.12.2; TOOLS_VERSION=2.2.0;;
2.5.2-1) WEBCFACE_VERSION=2.5.2; WEBUI_VERSION=1.11.0; TOOLS_VERSION=2.1.3;;
2.5.2)   WEBCFACE_VERSION=2.5.2; WEBUI_VERSION=1.10.4; TOOLS_VERSION=2.1.3;;
2.5.1)   WEBCFACE_VERSION=2.5.1; WEBUI_VERSION=1.10.3; TOOLS_VERSION=2.1.3;;
2.5.0-2) WEBCFACE_VERSION=2.5.0; WEBUI_VERSION=1.10.2; TOOLS_VERSION=2.1.2;;
2.5.0-1) WEBCFACE_VERSION=2.5.0; WEBUI_VERSION=1.10.1; TOOLS_VERSION=2.1.2;;
2.5.0)   WEBCFACE_VERSION=2.5.0; WEBUI_VERSION=1.10.0; TOOLS_VERSION=2.1.2;;
2.4.2)   WEBCFACE_VERSION=2.4.2; WEBUI_VERSION=1.10.0; TOOLS_VERSION=2.1.1;;
2.4.1-2) WEBCFACE_VERSION=2.4.1; WEBUI_VERSION=1.10.0; TOOLS_VERSION=2.1.1;;
2.4.1-1) WEBCFACE_VERSION=2.4.1; WEBUI_VERSION=1.10.0; TOOLS_VERSION=2.1.0;;
2.4.1)   WEBCFACE_VERSION=2.4.1; WEBUI_VERSION=1.9.1;  TOOLS_VERSION=2.1.0;;
2.4.0-1) WEBCFACE_VERSION=2.4.0; WEBUI_VERSION=1.9.0;  TOOLS_VERSION=2.1.0;;
2.4.0)   WEBCFACE_VERSION=2.4.0; WEBUI_VERSION=1.9.0;  TOOLS_VERSION=2.0.1;;
2.3.0)   WEBCFACE_VERSION=2.3.0; WEBUI_VERSION=1.8.3;  TOOLS_VERSION=2.0.1;;
2.2.1)   WEBCFACE_VERSION=2.2.1; WEBUI_VERSION=1.8.2;  TOOLS_VERSION=2.0.1;;
2.2.0)   WEBCFACE_VERSION=2.2.0; WEBUI_VERSION=1.8.1;  TOOLS_VERSION=2.0.1;;
2.1.0)   WEBCFACE_VERSION=2.1.0; WEBUI_VERSION=1.8.1;  TOOLS_VERSION=2.0.1;;
2.0.5)   WEBCFACE_VERSION=2.0.5; WEBUI_VERSION=1.8.0;  TOOLS_VERSION=2.0.1;;
2.0.4)   WEBCFACE_VERSION=2.0.4; WEBUI_VERSION=1.8.0;  TOOLS_VERSION=2.0.1;;
*)
    echo "Invalid version: $VERSION"
    echo
    usage
    exit 1
    ;;
esac

if [ -n "$*" ]; then
    usage
fi

case $(uname -s) in
Linux)
    case $(uname -m) in
    x86_64|x64)
        ARCH=amd64
        WEBUI_ARCH=amd64
        LIBPATH=x86_64-linux-gnu
        ;;
    aarch64|arm64|armv8l)
        ARCH=arm64
        WEBUI_ARCH=arm64
        LIBPATH=aarch64-linux-gnu
        ;;
    arm|armv7l)
        ARCH=armhf
        WEBUI_ARCH=armv7l
        LIBPATH=arm-linux-gnueabihf
        ;;
    *)
        echo "Unsupported architecture: $(uname -m)"
        exit 1
        ;;
    esac

    if [ $(id -u) -ne 0 ]; then
        echo "Warning: This install script may fail without sudo, depending on the installation destination."
    fi
    if [ -n "$OPT_USER" ]; then
        echo "Warning: -u option not supported on Linux."
    fi

    if type apt-get >/dev/null 2>&1; then
        if apt list --installed 'webcface*' 2>&1 | grep -q '^webcface'; then
            # WebCFace is already installed with apt
            case "$OPT_APT" in
            ""|1)
                echo "Note: WebCFace is already installed with apt, so it will be re-installed (or upgraded.)"
                ;;
            0)
                echo "Error: WebCFace is already installed with apt. Cannot manually extract over that."
                exit 1
                ;;
            esac
            OPT_APT=1
        else
            # apt was found but WebCFace is not installed
            case "$OPT_APT" in
            "")
                if [ -n "$OPT_DIR" ]; then
                    # echo "Note: apt-get found but will not use it since -d is specified."
                    OPT_APT=0
                else
                    if [ -e /opt/webcface ]; then
                        echo "Note: /opt/webcface already exists. Cannot install WebCFace with apt."
                        OPT_APT=0
                    else
                        echo "Note: if you run this in non-interactive scripts, pass -a option to force installing with apt, or -x otherwise."
                        printf "%s" "Do you want to install WebCFace with apt (recommended)? [Y/n] "
                        if [ -n "$OPT_YES" ]; then
                            echo
                            echo "Warning: apt will be used because -y option is specified."
                            echo
                            OPT_APT=1
                        else
                            read yn
                            case $yn in
                            [nN]*) OPT_APT=0 ;;
                            *) OPT_APT=1 ;;
                            esac
                            echo
                        fi
                    fi
                fi
                ;;
            1)
                if [ -n "$OPT_DIR" ]; then
                    echo "Error: -a and -d options are mutually exclusive."
                    echo
                    usage
                fi
                if [ -e /opt/webcface ]; then
                    echo "Error: /opt/webcface already exists. Cannot install WebCFace with apt."
                    exit 1
                fi
                ;;
            esac
        fi
    else
        # apt was not found
        case "$OPT_APT" in
        "")
            echo "Note: apt-get not found. Will install manually."
            OPT_APT=0
            ;;
        1)
            echo "Error: apt-get not found. Cannot install with apt."
            exit 1
            ;;
        esac
    fi
    case "$OPT_APT" in
    0)
        if [ -e "$INSTALL_DIR" ]; then
            echo "Warning: $INSTALL_DIR already exists. It will be overwritten."
        fi
        printf "%s" "This script will install WebCFace $VERSION (webui: $WEBUI_VERSION, tools: $TOOLS_VERSION) to $INSTALL_DIR. OK to proceed? [Y/n] "
        if [ -z "$OPT_YES" ]; then
            read yn
            case $yn in
            [nN]*) exit 1;;
            esac
        fi
        echo
        ZIP=webcface_${VERSION}_linux_$ARCH.zip
        echo "Downloading $ZIP..."
        curl -fL -o /tmp/$ZIP https://github.com/na-trium-144/webcface-package/releases/download/v$VERSION/$ZIP
        echo "Extracting $ZIP to $INSTALL_DIR..."
        unzip -o /tmp/$ZIP -d $INSTALL_DIR
        rm -f /tmp/$ZIP
        echo
        echo "Done."
        echo "You may need to add the following lines to your .bashrc or .zshrc etc.:"
        echo "  export PATH=\"$INSTALL_DIR\"/bin:\$PATH\""
        echo "  export PKG_CONFIG_PATH=\"$INSTALL_DIR/lib/$LIBPATH/pkgconfig:\$PKG_CONFIG_PATH\""
        ;;
    1)
        printf "%s" "This script will install WebCFace $VERSION (webui: $WEBUI_VERSION, tools: $TOOLS_VERSION) using apt. OK to proceed? [Y/n] "
        if [ -z "$OPT_YES" ]; then
            read yn
            case $yn in
            [nN]*) exit 1;;
            esac
        fi
        echo
        WEBCFACE_DEB=webcface_${WEBCFACE_VERSION}_$ARCH.deb
        echo "Downloading $WEBCFACE_DEB..."
        curl -fL -o /tmp/$WEBCFACE_DEB https://github.com/na-trium-144/webcface/releases/download/v$WEBCFACE_VERSION/$WEBCFACE_DEB
        TOOLS_DEB=webcface-tools_${TOOLS_VERSION}_$ARCH.deb
        echo "Downloading $TOOLS_DEB..."
        curl -fL -o /tmp/$TOOLS_DEB https://github.com/na-trium-144/webcface-tools/releases/download/v$TOOLS_VERSION/$TOOLS_DEB
        WEBUI_DEB=webcface-webui_${WEBUI_VERSION}_all.deb
        echo "Downloading $WEBUI_DEB..."
        curl -fL -o /tmp/$WEBUI_DEB https://github.com/na-trium-144/webcface-webui/releases/download/v$WEBUI_VERSION/$WEBUI_DEB
        DESKTOP_DEB=webcface-desktop_${WEBUI_VERSION}_linux_$WEBUI_ARCH.deb
        echo "Downloading $DESKTOP_DEB..."
        curl -fL -o /tmp/$DESKTOP_DEB https://github.com/na-trium-144/webcface-webui/releases/download/v$WEBUI_VERSION/$DESKTOP_DEB
        if [ -n "$OPT_YES" ]; then
            APT_Y=-y
        fi
        echo "apt-get reinstall $APT_Y /tmp/$WEBCFACE_DEB /tmp/$TOOLS_DEB /tmp/$WEBUI_DEB /tmp/$DESKTOP_DEB"
        apt-get reinstall $APT_Y /tmp/$WEBCFACE_DEB /tmp/$TOOLS_DEB /tmp/$WEBUI_DEB /tmp/$DESKTOP_DEB
        rm -f /tmp/$WEBCFACE_DEB /tmp/$TOOLS_DEB /tmp/$WEBUI_DEB /tmp/$DESKTOP_DEB
        echo
        echo "Done."
        echo "Since symlinks to WebCFace are created in /usr using apt-get, so there is no need to modify your PATH."
    esac
    ;;
Darwin)
    if [ $(id -u) -ne 0 ]; then
        echo "Warning: This install script may fail without sudo, depending on the installation destination."
        if [ -n "$OPT_USER" ]; then
            APP_INSTALL_DIR=$HOME/Applications
        fi
    else
        if [ -n "$OPT_USER" ]; then
            echo "Error: -u option must be used without sudo."
            exit 1
        fi
    fi
    if [ "$OPT_APT" = 1 ]; then
        echo "Warning: -a option not supported on macOS."
    fi

    if type brew >/dev/null 2>&1; then
        if [ -n "$SUDO_USER" ]; then
            # Prevent Homebrew from running as root
            brew="sudo -u $SUDO_USER brew"
        else
            brew=brew
        fi
        if $brew list --versions webcface >/dev/null; then
            echo "Error: WebCFace is already installed with Homebrew. Please uninstall it first."
            exit 1
        fi
    fi
    if [ -e "$INSTALL_DIR" ]; then
        echo "Warning: $INSTALL_DIR already exists. It will be overwritten."
    fi
    printf "%s" "This script will install WebCFace $VERSION (webui: $WEBUI_VERSION, tools: $TOOLS_VERSION) to $INSTALL_DIR and $APP_INSTALL_DIR. OK to proceed? [Y/n] "
    if [ -z "$OPT_YES" ]; then
        read yn
        case $yn in
        [nN]*) exit 1;;
        esac
    fi
    echo
    ZIP=webcface_${VERSION}_macos_universal.zip
    echo "Downloading $ZIP..."
    curl -fL -o /tmp/$ZIP https://github.com/na-trium-144/webcface-package/releases/download/v${VERSION}/$ZIP
    APP_ZIP=webcface-desktop_${VERSION}_macos_app.zip
    echo "Downloading $APP_ZIP..."
    curl -fL -o /tmp/$APP_ZIP https://github.com/na-trium-144/webcface-package/releases/download/v${VERSION}/$APP_ZIP
    echo "Extracting $ZIP to $INSTALL_DIR..."
    unzip -o /tmp/$ZIP -d $INSTALL_DIR
    rm -f /tmp/$ZIP
    echo "Extracting $APP_ZIP to $APP_INSTALL_DIR..."
    unzip -o /tmp/$APP_ZIP -d $APP_INSTALL_DIR
    rm -f /tmp/$APP_ZIP
    echo
    echo "Done."
    echo "You may need to add the following lines to your .bashrc or .zshrc etc.:"
    echo "  export PATH=\"$INSTALL_DIR/bin:\$PATH\""
    echo "  export PKG_CONFIG_PATH=\"$INSTALL_DIR/lib/pkgconfig:\$PKG_CONFIG_PATH\""
    if [ "$INSTALL_DIR" != "/opt/webcface" ]; then
        echo "  export DYLD_LIBRARY_PATH=\"$INSTALL_DIR/lib:\$DYLD_LIBRARY_PATH\""
    fi
    ;;
*)
    echo "Unsupported OS: $(uname -s)"
    exit 1
    ;;
esac

