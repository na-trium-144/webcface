import { pack, unpack } from "./message.js";
import * as types from "./messageType.js";
import { ClientData } from "./clientData.js";
import { DataStore, emptyStore, Value, Text } from "./data.js";
import { Func, FuncResult, FuncInfoInternal } from "./func.js";
import websocket from "websocket";
const w3cwebsocket = websocket.w3cwebsocket;

export class Client {
  data: ClientData;
  ws: null | websocket.w3cwebsocket = null;
  connected = false;
  host: string;
  port: number;
  syncInit = false;
  constructor(name: string, host = "127.0.0.1", port = 7530) {
    this.data = new ClientData(name);
    this.host = host;
    this.port = port;
    this.reconnect();
  }
  reconnect() {
    let connection_done = false;
    const ws = new w3cwebsocket(`ws://${this.host}:${this.port}`);
    this.ws = ws;
    setTimeout(() => {
      if (!connection_done) {
        ws.onopen = () => null;
        ws.onmessage = () => null;
        ws.onclose = () => null;
        ws.onerror = () => null;
        this.ws = null;
        this.reconnect();
      }
    }, 1000);
    ws.binaryType = "arraybuffer";
    ws.onopen = () => {
      connection_done = true;
      this.connected = true;
      ws.send(pack(types.kind.name, { n: this.data.selfMemberName }));
    };
    ws.onmessage = (event) => {
      const [kind, data] = unpack(event.data as ArrayBuffer);
      switch (kind) {
        case types.kind.recv + types.kind.value: {
          const dataR = data as types.Recv;
          this.data.valueStore.setRecv(dataR.f, dataR.n, dataR.d);
          break;
        }
        case types.kind.recv + types.kind.text: {
          const dataR = data as types.Recv;
          this.data.textStore.setRecv(dataR.f, dataR.n, dataR.d);
          break;
        }
        case types.kind.call: {
          const dataR = data as types.Call;
          const r: types.CallResponse = {
            i: dataR.i,
            c: dataR.c,
            f: false,
            e: false,
            r: "",
          };
          const s = this.funcStore.dataRecv.get("");
          if (s) {
            const m = s.get(dataR.n);
            if (m) {
              r.f = true;
              try {
                if (m.funcImpl != null) {
                  r.r = m.funcImpl(...dataR.a);
                }
              } catch (e: any) {
                r.r = (e as Error).toString();
                r.e = true;
              }
            } else {
              r.f = false;
            }
          } else {
            r.f = false;
          }
          this.ws != null && this.ws.send(pack(types.kind.callResponse, r));
          break;
        }
        case types.kind.callResponse: {
          const dataR = data as types.CallResponse;
          const r = this.funcResult[dataR.i];
          r.found = dataR.f;
          r.isError = dataR.e;
          if (r.isError) {
            r.errorMsg = String(dataR.r);
          } else {
            r.result = dataR.r;
          }
          r.ready = true;
          break;
        }
      }
    };
    ws.onerror = (error) => {
      connection_done = true;
      console.error("Connection Error");
      // console.error(error);
    };
    ws.onclose = () => {
      connection_done = true;
      this.connected = false;
      console.error("closed");
      setTimeout(() => this.reconnect(), 1000);
    };
  }
  sync() {
    if (this.connected && this.ws != null) {
      for (const [k, v] of this.valueStore.dataSend.entries()) {
        this.ws.send(pack(types.kind.value, { n: k, d: v }));
      }
      this.valueStore.dataSend.clear();
      for (const [k, v] of this.valueStore.reqSend.entries()) {
        for (const [k2, v2] of v.entries()) {
          this.ws.send(
            pack(types.kind.subscribe + types.kind.value, { f: k, n: k2 })
          );
        }
      }
      this.valueStore.reqSend.clear();

      for (const [k, v] of this.textStore.dataSend.entries()) {
        this.ws.send(pack(types.kind.text, { n: k, d: v }));
      }
      this.textStore.dataSend.clear();
      for (const [k, v] of this.textStore.reqSend.entries()) {
        for (const [k2, v2] of v.entries()) {
          this.ws.send(
            pack(types.kind.subscribe + types.kind.text, { f: k, n: k2 })
          );
        }
      }
      this.textStore.reqSend.clear();
    }
  }
  subject(name: string) {
    return new SubjectClient(this, name);
  }
  subjects() {
    return [...this.valueStore.entry.keys()].map((n) => this.subject(n));
  }
  value(name: string) {
    return new Value(this.valueStore, "", name);
  }
  text(name: string) {
    return new Text(this.textStore, "", name);
  }
  func(name: string) {
    return new Func(
      (m: ArrayBuffer) => this.connected && this.ws != null && this.ws.send(m),
      this.funcStore,
      this.funcResult,
      "",
      name
    );
  }
}
