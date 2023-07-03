import { pack, unpack } from "./message";
import * as types from "./messageType";
import { Value, Text } from "./data";
import { Func, FuncResult, FuncStore } from "./func";
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
  funcStore: FuncStore[] = [];
  funcResult: FuncResult[] = [];
  name: string;
  host: string;
  port: number;
  constructor(name: string, host = "", port = 80) {
    this.name = name;
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
        console.error("timeout!");
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
      ws.send(pack(types.kind.name, { n: this.name }));
    };
    ws.onmessage = (event) => {
      const [kind, data] = unpack(event.data as ArrayBuffer);
      switch (kind) {
        case types.kind.recv + types.kind.value: {
          const dataR = data as types.Recv;
          const i = this.valueRecv.findIndex(
            (s) => s.f == dataR.f && s.n == dataR.n
          );
          if (i >= 0) {
            this.valueRecv[i] = dataR;
          } else {
            this.valueRecv.push(dataR);
          }
          break;
        }
        case types.kind.recv + types.kind.text: {
          const dataR = data as types.Recv;
          const i = this.textRecv.findIndex(
            (s) => s.f == dataR.f && s.n == dataR.n
          );
          if (i >= 0) {
            this.textRecv[i] = dataR;
          } else {
            this.textRecv.push(dataR);
          }
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
          const s = this.funcStore.find((s) => s.name === dataR.n);
          if (s) {
            r.f = true;
            try {
              r.r = String(s.func(...dataR.a));
            } catch (e: any) {
              r.r = (e as Error).toString();
              r.e = true;
            }
          } else {
            r.f = false;
          }
          this.ws != null && this.ws.send(pack(types.kind.callResponse, r));
        }
        case types.kind.callResponse: {
          const dataR = data as types.CallResponse;
          const r = this.funcResult[dataR.i];
          r.found = dataR.f;
          r.isError = dataR.e;
          if (r.isError) {
            r.errorMsg = dataR.r;
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
  text(from: string, name?: string) {
    if (name == undefined) {
      [from, name] = ["", from];
    }
    return new Text(this.textSend, this.textSubsc, this.textRecv, from, name);
  }
  func(from: string, name?: string) {
    if (name == undefined) {
      [from, name] = ["", from];
    }
    return new Func(
      (m: ArrayBuffer) => this.connected && this.ws != null && this.ws.send(m),
      this.funcStore,
      this.funcResult,
      from,
      name
    );
  }
}
