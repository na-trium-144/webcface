function looseJsonParse(obj) {
  try {
    return Function('"use strict";return (' + obj + ")")();
  } catch {
    return obj;
  }
}
const ws = new WebSocket(document.URL.replace("http", "ws") + "ws");
ws.addEventListener("message", function (event) {
  // console.log(event.data);
  const msgJson = JSON.parse(event.data);
  const msgName = msgJson.msgname;
  const msgContent = msgJson.msg;
  if (msgName === "setting") {
    document.getElementById("setting").innerText = JSON.stringify(
      msgContent,
      null,
      2
    );
    const dom1 = document.getElementById("call");
    dom1.innerHTML = "";
    for (let f of msgContent.functions) {
      dom1.innerHTML += `
      <p>
        <button onclick='call("${f.name}", ${f.args.length})'>${f.name}</button>
        ${f.args.map(
          (a, i) =>
            `${a.name} = <input size=10 id="${f.name}::${i}" name="${a.name}" placeholder="${a.type}" /> `
        )}
      </p>
      `;
    }
    const dom2 = document.getElementById("to_robot");
    dom2.innerHTML = "";
    for (let f of msgContent.to_robot) {
      dom2.innerHTML += `
      <p>
        <button onclick='to_robot("${f.name}", ${f.value.length})'>${
        f.name
      }</button>
        ${f.value.map(
          (a, i) =>
            `${a.name} = <input size=10 id="${f.name}::${i}" name="${a.name}" placeholder="${a.type}" /> `
        )}
      </p>
      `;
    }
    const dom3 = document.getElementById("from_robot");
    dom3.innerHTML = "";
    for (let f of msgContent.from_robot) {
      dom3.innerHTML += `
      <p>
      <span>${f.name}</span>
      <span id="${f.name}" ></span>
      </p>
      `;
    }
      dom3.innerHTML += `
      <p>
      <span>timestamp</span>
      <span id="timestamp" ></span>
      </p>
      `;
  }
  if (msgName === "from_robot") {
    for (let name in msgContent) {
      let obj = looseJsonParse(msgContent[name]);
      let outspan = document.getElementById(`${name}`);
      outspan.innerText = JSON.stringify(obj);
    }
  }
  if (msgName === "log") {
    document.getElementById("stream").innerText += msgContent.reduce(
      (prev, l) => (prev + `${new Date(l.timestamp).toLocaleTimeString()} ${l.level} ${l.text}\n`),
      ""
    );
  }
  if (msgName === "error") {
    document.getElementById("error").innerText = msgContent;
  }
});

function call(name, args_num) {
  sendobj = {
    name: name,
    args: {},
  };
  for (let i = 0; i < args_num; i++) {
    const arginput = document.getElementById(`${name}::${i}`);
    sendobj.args[arginput.name] = looseJsonParse(arginput.value);
  }
  console.log(JSON.stringify(sendobj));
  ws.send(JSON.stringify({ msgname: "function", msg: sendobj }));
}

function to_robot(name, args_num) {
  sendobj = {
    name: name,
    value: {},
  };
  for (let i = 0; i < args_num; i++) {
    const arginput = document.getElementById(`${name}::${i}`);
    sendobj.value[arginput.name] = looseJsonParse(arginput.value);
  }
  console.log(JSON.stringify(sendobj));
  ws.send(JSON.stringify({ msgname: "to_robot", msg: sendobj }));
}
