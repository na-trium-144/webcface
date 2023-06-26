import { pack, unpack } from "./message";
import * as types from "./messageType";
import { Value } from "./data";
import { w3cwebsocket } from "websocket";

export class Client {
  ws: null | w3cwebsocket = null;
  connected = false;
  valueSend: types.Data[] = [];
  valueSubsc: types.Subscribe[] = [];
  valueRecv: types.Recv[] = [];
  textSend: types.Data[] = [];
  textSubsc: types.Subscribe[] = [];
  textRecv: types.Recv[] = [];
  constructor(name: string, host = "", port = 80) {
    const ws = new w3cwebsocket(`ws://${host}:${port}`);
    this.ws = ws;
    ws.binaryType = "arraybuffer";
    ws.onopen = () => {
      this.connected = true;
      ws.send(pack(types.kind.name, { n: name }));
    };
    ws.onmessage = (event) => {
      const [kind, data] = unpack(event.data as ArrayBuffer);
      switch (kind) {
        case types.kind.recv + types.kind.value:
          this.valueRecv.push(data as types.Recv);
          break;
        case types.kind.recv + types.kind.text:
          this.textRecv.push(data as types.Recv);
          break;
      }
    };
    ws.onerror = (error) => {
      console.error("Connection Error");
      // console.error(error);
    };
    ws.onclose = () => {
      this.connected = false;
    };
  }
  send() {
    if (this.connected && this.ws != null) {
      for (const v of this.valueSend) {
        this.ws.send(pack(types.kind.value, v));
      }
      for (const v of this.valueSubsc) {
        this.ws.send(pack(types.kind.subscribe + types.kind.value, v));
      }
      for (const v of this.textSend) {
        this.ws.send(pack(types.kind.text, v));
      }
      for (const v of this.textSubsc) {
        this.ws.send(pack(types.kind.subscribe + types.kind.text, v));
      }
    }
  }
  value(from: string, name?: string) {
    if (name == undefined) {
      [from, name] = ["", from];
    }
    return new Value(
      this.valueSend,
      this.valueSubsc,
      this.valueRecv,
      from,
      name
    );
  }
  text() {
    console.log("hoge");
  }
  func() {
    console.log("hoge");
  }
}
