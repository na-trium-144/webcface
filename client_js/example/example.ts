import { Client } from "../src/index";

const c = new Client("example_ts", "127.0.0.1", 80);

setInterval(() => {
  console.log(`recv test = ${c.value("example_main", "test").get()}`);
  void c.func("example_main", "func2").run("9", "7", "0", "").then((v) => {
    console.log(`func2 = ${v.result}`); // vだけで値取れるようにしない?
  }); // .catchできるようにしない?
  c.send();
}, 250);
