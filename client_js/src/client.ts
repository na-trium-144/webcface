import { pack, unpack } from "./message.js";
import * as types from "./messageType.js";
import { ClientData } from "./clientData.js";
import { Member } from "./member.js";
import { FieldBase, Value, Text } from "./data.js";
import { Func, runFunc } from "./func.js";
import { Val, AsyncFuncResult } from "./funcResult.js";
import websocket from "websocket";
const w3cwebsocket = websocket.w3cwebsocket;

export class Client extends Member {
  ws: null | websocket.w3cwebsocket = null;
  connected = false;
  host: string;
  port: number;
  syncInit = false;
  constructor(name: string, host = "127.0.0.1", port = 7530) {
    super(new FieldBase(new ClientData(name), name), name);
    this.host = host;
    this.port = port;
    this.reconnect();
  }
  send(kind: number, obj: types.Any) {
    if (this.ws != null) {
      this.ws.send(pack(kind, obj));
    }
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
    };
    ws.onmessage = (event) => {
      const [kind, data] = unpack(event.data as ArrayBuffer);
      switch (kind) {
        case types.kind.value: {
          const dataR = data as types.Data<number>;
          this.data.valueStore.setRecv(dataR.m, dataR.n, dataR.d);
          break;
        }
        case types.kind.text: {
          const dataR = data as types.Data<string>;
          this.data.textStore.setRecv(dataR.m, dataR.n, dataR.d);
          break;
        }
        case types.kind.call: {
          setTimeout(() => {
            const dataR = data as types.Call;
            const s = this.data.funcStore.dataRecv.get(
              this.data.selfMemberName
            );
            if (s) {
              const m = s.get(dataR.n);
              if (m) {
                this.send(types.kind.callResponse, {
                  i: dataR.i,
                  c: dataR.c,
                  s: true,
                });
                try {
                  if (m.funcImpl != null) {
                    const res = runFunc(m, dataR.a);
                    this.send(types.kind.callResult, {
                      i: dataR.i,
                      c: dataR.c,
                      e: false,
                      r: res == undefined ? "" : res,
                    });
                  }
                } catch (e: any) {
                  this.send(types.kind.callResult, {
                    i: dataR.i,
                    c: dataR.c,
                    e: true,
                    r: (e as Error).toString(),
                  });
                }
              } else {
                this.send(types.kind.callResponse, {
                  i: dataR.i,
                  c: dataR.c,
                  s: false,
                });
              }
            } else {
              this.send(types.kind.callResponse, {
                i: dataR.i,
                c: dataR.c,
                s: false,
              });
            }
          });
          break;
        }
        case types.kind.callResponse: {
          const dataR = data as types.CallResponse;
          const r = this.data.funcResultStore.getResult(dataR.i);
          r.resolveStarted(dataR.s);
          break;
        }
        case types.kind.callResult: {
          const dataR = data as types.CallResult;
          const r = this.data.funcResultStore.getResult(dataR.i);
          if (dataR.e) {
            r.rejectResult(new Error(String(dataR.r)));
          } else {
            r.resolveResult(dataR.r);
          }
          break;
        }
        case types.kind.syncInit: {
          const dataR = data as types.SyncInit;
          this.data.valueStore.addMember(dataR.m);
          this.data.textStore.addMember(dataR.m);
          this.data.funcStore.addMember(dataR.m);
          break;
        }
        case types.kind.entry + types.kind.value: {
          const dataR = data as types.Entry;
          this.data.valueStore.setEntry(dataR.m, dataR.n);
          break;
        }
        case types.kind.entry + types.kind.text: {
          const dataR = data as types.Entry;
          this.data.textStore.setEntry(dataR.m, dataR.n);
          break;
        }
        case types.kind.funcInfo: {
          const dataR = data as types.FuncInfo;
          this.data.funcStore.setEntry(dataR.m, dataR.n);
          this.data.funcStore.setRecv(dataR.m, dataR.n, {
            returnType: dataR.r,
            args: dataR.a.map((a) => ({
              name: a.n,
              type: a.t,
              init: a.i,
              min: a.m,
              max: a.x,
              option: a.o,
            })),
            call: (r: AsyncFuncResult, args: Val[]) => {
              this.send(types.kind.call, {
                i: r.callerId,
                c: r.caller,
                r: dataR.m,
                n: dataR.n,
                a: args,
              });
            },
          });
          break;
        }
      }
    };
    ws.onerror = (error: any) => {
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
      if (!this.syncInit) {
        this.send(types.kind.syncInit, { m: this.data.selfMemberName });
        this.syncInit = true;
      }

      for (const [k, v] of this.data.valueStore.transferSend().entries()) {
        this.send(types.kind.value, { m: "", n: k, d: v });
      }
      for (const [k, v] of this.data.valueStore.transferReq().entries()) {
        for (const [k2, v2] of v.entries()) {
          this.send(types.kind.subscribe + types.kind.value, { f: k, n: k2 });
        }
      }

      for (const [k, v] of this.data.textStore.transferSend().entries()) {
        this.send(types.kind.text, { m: "", n: k, d: v });
      }
      for (const [k, v] of this.data.textStore.transferReq().entries()) {
        for (const [k2, v2] of v.entries()) {
          this.send(types.kind.subscribe + types.kind.text, { f: k, n: k2 });
        }
      }

      for (const [k, v] of this.data.funcStore.transferSend().entries()) {
        this.send(types.kind.funcInfo, {
          m: "",
          n: k,
          r: v.returnType,
          a: v.args.map((a) => ({
            n: a.name || "",
            t: a.type != undefined ? a.type : types.argType.none_,
            i: a.init != undefined ? a.init : null,
            m: a.min != undefined ? a.min : null,
            x: a.max != undefined ? a.max : null,
            o: a.option != undefined ? a.option : [],
          })),
        });
      }
    }
  }
  member(member: string) {
    return new Member(this, member);
  }
  members() {
    return [...this.data.valueStore.getMembers()].map((n) => this.member(n));
  }
}
