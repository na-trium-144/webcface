from . import pywebcface
import os


def start_server(port: int = 80, static_dir: str = ""):
    if static_dir == "":
        static_dir = os.path.join(
            os.path.dirname(__file__),
            "..",
            "..",
            "..",
            "..",
            "share",
            "webcface",
            "frontend",
            "out",
        )
    pywebcface.start_server(port, static_dir)


quit_server = pywebcface.quit_server
send_data = pywebcface.send_data
set_server_name = pywebcface.set_server_name
add_related_server = pywebcface.add_related_server
