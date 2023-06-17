import pytest
import websocket
import webcface
import time
import threading
import json

var1 = 333
var2 = 66.66
var3 = True
var4 = "hello"
var5 = [1, 2, 3]

port = 60000

received = 0


def check_var1(ws):
    global received
    while True:
        msg = json.loads(ws.recv())
        print(msg)
        if msg["msgname"] == "from_robot":
            if "status1" in msg["msg"]:
                assert msg["msg"]["status1"] == var1
                received += 1
            if "status2" in msg["msg"]:
                assert msg["msg"]["status2"] == var2
                received += 1
            if "status3" in msg["msg"]:
                assert msg["msg"]["status3"] == var3
                received += 1
            if "status4" in msg["msg"]:
                assert msg["msg"]["status4"] == var4
                received += 1
            if "status5" in msg["msg"]:
                assert msg["msg"]["status5"] == var5
                received += 1
            if "var1" in msg["msg"]:
                assert msg["msg"]["var1"] == var1
                received += 1
            if "var2" in msg["msg"]:
                assert msg["msg"]["var2"] == var2
                received += 1
            if "var3" in msg["msg"]:
                assert msg["msg"]["var3"] == var3
                received += 1
            if "var4" in msg["msg"]:
                assert msg["msg"]["var4"] == var4
                received += 1
            if "var5" in msg["msg"]:
                assert msg["msg"]["var5"] == var5
                received += 1


def test_websocket_from_robot():
    global var1, var2, var3, var4, var5, received
    webcface.start_server(port)
    webcface.add_function_from_robot("status1", lambda: var1, int)
    webcface.add_function_from_robot("status2", lambda: var2, float)
    webcface.add_function_from_robot("status3", lambda: var3, bool)
    webcface.add_function_from_robot("status4", lambda: var4, str)
    webcface.add_function_from_robot("status5", lambda: var5, "list[int]")
    webcface.add_value_from_robot("var1", var1, int)
    webcface.add_value_from_robot("var2", var2, float)
    webcface.add_value_from_robot("var3", var3, bool)
    webcface.add_value_from_robot("var4", var4, str)
    webcface.add_value_from_robot("var5", var5, "list[int]")
    time.sleep(1)
    ws = websocket.create_connection(f"ws://localhost:{port}/")
    t1 = threading.Thread(target=check_var1, args=[ws])
    t1.start()
    webcface.send_data()
    time.sleep(1)
    assert received == 10
    var1 += 1
    var2 += 1
    var3 = not var3
    var4 = "hogehoge"
    var5 = [4, 5, 6]
    webcface.add_value_from_robot("var1", var1, int)
    webcface.add_value_from_robot("var2", var2, float)
    webcface.add_value_from_robot("var3", var3, bool)
    webcface.add_value_from_robot("var4", var4, str)
    webcface.add_value_from_robot("var5", var5, "list[int]")
    webcface.send_data()
    time.sleep(1)
    assert received == 20
    ws.close()

