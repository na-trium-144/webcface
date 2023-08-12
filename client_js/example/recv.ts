import { Client } from "../src/index.js";

const c = new Client("example_recv");

setInterval(() => {
  console.log(`recv test = ${c.member("example_main").value("test").get()}`);
  // 数値を引数に渡せるようにしない?
  void c
    .member("example_main")
    .func("func2")
    .run(9, 7.1, false, "aaa")
    .result.then((v) => {
      console.log(`func2 = ${v as number}`); // vだけで値取れるようにしない?
    }); // .catchできるようにしない?
  c.sync();
}, 250);
