import { Client, valType } from "../src/index.js";

const c = new Client("example_main");
c.value("test").set(0);
const f1 = c
  .func("func1")
  .set(() => console.log("hello, world!"), valType.none_, []);
c.func("func2").set(
  (a: number, b: number, c: boolean, d: string) => {
    console.log(`hello world 2 ${a} ${b} ${c ? "true" : "false"} ${d}`);
    return a + b;
  },
  valType.float_,
  [
    { name: "a", type: valType.int_, init: 3 },
    { name: "b", type: valType.float_, min: 2, max: 8 },
    { name: "c", type: valType.boolean_, init: false },
    { name: "d", type: valType.string_, option: ["hoge", "fuga"] },
  ]
);

setInterval(() => {
  c.value("test").set(c.value("test").get() + 1);
  c.text("str").set("hello");
  console.log(`str = ${c.member("example_main").text("str").get()}`);
  console.log(`test = ${c.member("example_main").value("test").get()}`);
  void c.func("func1").runAsync();
  void c
    .func("func2")
    .runAsync(3, 5.5, 1, "hoge")
    .result.then((v) => console.log(`return = ${v as number}`));
  c.sync();
}, 250);
