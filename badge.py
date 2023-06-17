import toml
import os
project = toml.load(os.path.join(os.path.dirname(__file__), "pyproject.toml"))["tool"]["poetry"]
import anybadge
badge = anybadge.Badge("webcface", project["version"])
badge.write_badge("version.svg")
