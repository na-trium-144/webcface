from skbuild import setup
import platform
import toml
import os
project = toml.load(os.path.join(os.path.dirname(__file__), "pyproject.toml"))["tool"]["poetry"]
setup(
    name=project["name"],
    version=project["version"],
    description=project["description"],
    author=project["authors"],
    # license="MIT",
    packages=['webcface'],
    scripts=["python/webcface-starter"],
    package_dir={"": "python"},
    python_requires=">=3.7",
    cmake_args=[
        "-DWEBCFACE_PYTHON_INSTALL=true",
        f"-DPYBIND11_PYTHON_VERSION={platform.python_version()}",
    ],
    install_requires=["toml", "psutil"],
)
