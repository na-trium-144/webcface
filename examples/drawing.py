import webcface
import time

webcface.start_server()


def hello():
    print("hello")


i = 0
while True:
    i += 1
    p = webcface.PageLayout("testpage")
    p.clear()
    p.add("drawing.py")
    p.add(12345)
    p.add("aaaaa")
    p.add(True)
    p.new_line()
    p.add("i = ").add(webcface.RegisterValue("i", ret=int, value=i)).new_line()
    p.add(webcface.Button("hello", webcface.RegisterCallback("hello", arg={}, callback=hello))).new_line()
    d = webcface.Drawing(300, 300)
    dl = d.create_layer()
    dl.draw_rect(0, 0, 300, 300, "beige")
    dl.draw_circle(140, 140, 130, "aqua")
    dl.draw_circle(140, 140, 60, "white")
    dl.draw_circle(200, 200, 90, "yellow")
    dl.draw_circle(200, 200, 40, "white")
    p.add(d)
    webcface.send_data()
    time.sleep(0.1)
