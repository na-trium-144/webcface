import { Client } from "../src/index.js";

const c = new Client("example_recv", "127.0.0.1", 80);

setInterval(() => {
  console.log(`recv test = ${c.subject("example_main").value("test").get()}`);
  // 数値を引数に渡せるようにしない?
  void c
    .subject("example_main")
    .func("func2")
    .run(9, 7.1, false, "aaa")
    .then((v) => {
      console.log(`func2 = ${v.result}`); // vだけで値取れるようにしない?
    }); // .catchできるようにしない?
  c.send();
}, 250);
