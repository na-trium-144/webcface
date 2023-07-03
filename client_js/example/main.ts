import { Client } from "../src/index";

const c = new Client("example_main", "127.0.0.1", 80);
c.value("test").set(0);
c.func("func1").set(() => console.log("hello, world!"));
c.func("func2").set((a: string, b: string, c: string, d: string) => {
  console.log(`hello world 2 ${a} ${b} ${c} ${d}`);
  return (parseInt(a) + parseFloat(b)).toString();
});

setInterval(() => {
  c.value("test").set(c.value("test").get() + 1);
  c.text("str").set("hello");
  console.log(`str = ${c.text("example_main", "str").get()}`);
  console.log(`test = ${c.value("example_main", "test").get()}`);
  void c.func("func1").run();
  void c
    .func("func2")
    .run("3", "5.5", "1", "hoge")
    .then((v) => console.log(`return = ${v.result}`));
  c.send();
}, 250);
