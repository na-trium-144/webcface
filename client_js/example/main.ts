import { Client, argType } from "../src/index.js";

const c = new Client("example_main");
c.value("test").set(0);
const f1 = c
  .func("func1")
  .set(() => console.log("hello, world!"), argType.none_, []);
c.func("func2").set(
  (a: number, b: number, c: boolean, d: string) => {
    console.log(`hello world 2 ${a} ${b} ${c ? "true" : "false"} ${d}`);
    return a + b;
  },
  argType.float_,
  [
    { name: "a", type: argType.int_, init: 3 },
    { name: "b", type: argType.float_, min: 2, max: 8 },
    { name: "c", type: argType.boolean_, init: false },
    { name: "d", type: argType.string_, option: ["hoge", "fuga"] },
  ]
);

setInterval(() => {
  c.value("test").set(c.value("test").get() + 1);
  c.text("str").set("hello");
  console.log(`str = ${c.member("example_main").text("str").get()}`);
  console.log(`test = ${c.member("example_main").value("test").get()}`);
  void c.func("func1").run();
  void c
    .func("func2")
    .run(3, 5.5, 1, "hoge")
    .result.then((v) => console.log(`return = ${v as number}`));
  c.sync();
}, 250);
