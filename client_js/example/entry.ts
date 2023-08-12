import { Client } from "../src/index.js";

const c = new Client("example_get_entry", "127.0.0.1", 80);

setInterval(() => {
  for (const s of c.members()) {
    console.log(s.name);
    for (const v of s.values()) {
      console.log(`  value ${v.name}`);
    }
    for (const v of s.texts()) {
      console.log(`  text  ${v.name}`);
    }
    for (const v of s.funcs()) {
      console.log(
        `  func  ${v.name} arg: ${v.args.map((a) => `<${a.name} type=${a.type},init=${a.init},min=${a.min},max=${a.max},option=${a.option}`).join(", ")} ret: ${v.returnType}`
      );
    }
  }

  c.sync();
}, 250);
