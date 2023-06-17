from . import pywebcface
import sys
import io

# StdLogger(io_object)
#  write(text) でtextをwebcfaceとio_objectに送る
#  io_objectがNoneならwebcfaceのみに送る
class StdLogger(io.TextIOBase):
    def __init__(self, io_org):
        io.TextIOBase.__init__(self)
        self.io_org = io_org

    def write(self, text):
        pywebcface.append_log_line(0, text)
        if self.io_org is not None:
            self.io_org.write(text)


def init_sys_logger():
    sys.stdout = StdLogger(sys.stdout)
    sys.stderr = StdLogger(sys.stderr)
