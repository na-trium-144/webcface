import { Client } from "../src/index";

const c = new Client("example_get_entry", "127.0.0.1", 80);

setInterval(() => {
  for (const s of c.subjects()) {
    console.log(s.name());
    for (const v of s.values()) {
      console.log(`  value ${v.name}`);
    }
    for (const v of s.texts()) {
      console.log(`  text  ${v.name}`);
    }
  }

  c.send();
}, 250);
