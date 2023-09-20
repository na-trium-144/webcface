import { Client, Member, Value, Text, Func } from "../src/index.js";

const c = new Client("example_recv");

c.onMemberEntry.on((m: Member) => {
  console.log(`member ${m.name}`);
  m.onValueEntry.on((v: Value) => {
    console.log(`value ${v.name}`);
  });
  m.onTextEntry.on((v: Text) => {
    console.log(`text ${v.name}`);
  });
  m.onFuncEntry.on((v: Func) => {
    console.log(
      `func  ${v.name} arg: {${v.args
        .map(
          (a) =>
            `<${a.name} type=${a.type},init=${a.init},min=${a.min},max=${a.max},option=${a.option}>`
        )
        .join(", ")}} ret: ${v.returnType}`
    );
  });
});

c.sync();
setInterval(() => {
  console.log(`recv test = ${c.member("example_main").value("test").get()}`);
  void c
    .member("example_main")
    .func("func2")
    .runAsync(9, 7.1, false, "aaa")
    .result.then((v) => {
      console.log(`func2 = ${v as number}`);
    })
    .catch(() => undefined);
  c.sync();
}, 250);
