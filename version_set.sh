#!/bin/sh
set -e
if [ -z "$1" ]; then
	echo "usage: version_set.sh version"
	exit 1
else
	cd `dirname "$0"`
	newver="$1"
	sed -i -E "2,\$s/VERSION [0-9\.]+/VERSION ${newver}/" CMakeLists.txt
	sed -i -E "s/\"version\": \"[0-9\.]+\"/\"version\": \"${newver}\"/" frontend/package.json
	sed -i -E "s/version = \"[0-9\.]+\"/version = \"${newver}\"/" pyproject.toml
	# CMakeLists → #define WEBCFACE_VERSION
	#            → python/webcface/version.py が生成される
	# setup.pyはpyproject.tomlを読む
fi