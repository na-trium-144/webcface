import { Client, Member, Value, Text, Func } from "../src/index.js";

const c = new Client("example_get_entry");
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

// todo: 接続できるまで待機する関数を実装?
setInterval(() => c.sync(), 10)