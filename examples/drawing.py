import webcface
import time

webcface.start_server()

webcface.RegisterCallback(
    "test1",
    arg={"あ": str},
    default_value=["abcde"],
    callback=lambda x: print(f"test1 {x}"),
)
webcface.RegisterCallback("test2").arg({"あ": str}).default_value(["abcde"]).callback(
    lambda x: print(f"test2 {x}")
)


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
    p.add(webcface.Alert("alert")).add(webcface.Alert("info", "info"))
    p.add("i = ").add(webcface.RegisterValue("i", ret=int, value=i)).new_line()
    p.add(
        webcface.Button(
            "hello", webcface.RegisterCallback("hello", arg={}, callback=hello)
        )
    ).new_line()
    d = webcface.Drawing(300, 300)
    dl = d.create_layer("a") # ver0.9から名前が必要(なんでもよい)
    dl.draw_rect(0, 0, 300, 300, "beige")
    dl.draw_circle(140, 140, 130, "aqua")
    dl.draw_circle(140, 140, 60, "white")
    dl.draw_circle(200, 200, 90, "yellow")
    dl.draw_circle(200, 200, 40, "white")
    p.add(d)
    webcface.send_data()
    time.sleep(0.1)
