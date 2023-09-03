import { pack, unpack } from "./message.js";
import * as types from "./message.js";
import {
  Field,
  FieldBase,
  ClientData,
  AsyncFuncResult,
  FieldWithEvent,
  eventType,
} from "./clientData.js";
import { Member, runFunc } from "./data.js";
import { Val } from "./funcInfo.js";
import {
  log4jsLoggingEvent,
  log4jsLevels,
  log4jsLevelConvert,
} from "./logger.js";
import { getViewDiff, mergeViewDiff } from "./view.js";
import websocket from "websocket";
const w3cwebsocket = websocket.w3cwebsocket;
import util from "util";
import { getLogger } from "@log4js-node/log4js-api";

export class Client extends Member {
  ws: null | websocket.w3cwebsocket = null;
  connected = false;
  host: string;
  port: number;
  syncInit = false;
  closing = false;
  reconnectTimer: ReturnType<typeof setTimeout> | null = null;
  get loggerInternal() {
    const logger = getLogger("webcface");
    if (logger.level != null) {
      return logger;
    } else {
      // log4jsが使えないときのフォールバック
      return {
        debug(...args: any[]) {
          console?.log(...args);
        },
        warn(...args: any[]) {
          console?.warn(...args);
        },
        error(...args: any[]) {
          console?.error(...args);
        },
      };
    }
  }
  constructor(name: string, host = "127.0.0.1", port = 7530) {
    super(
      new Field(
        new ClientData(name, (r: AsyncFuncResult, b: FieldBase, args: Val[]) =>
          this.callFunc(r, b, args)
        ),
        name
      ),
      name
    );
    this.host = host;
    this.port = port;
    this.reconnect();
  }
  send(kind: number, obj: types.AnyMessage) {
    this.ws?.send(pack(kind, obj));
  }
  callFunc(r: AsyncFuncResult, b: FieldBase, args: Val[]) {
    this.send(types.kind.call, {
      i: r.callerId,
      c: r.caller,
      r: b.member_,
      f: b.field_,
      a: args,
    });
  }
  close() {
    this.closing = true;
    if (this.reconnectTimer != null) {
      clearTimeout(this.reconnectTimer);
    }
    this.ws?.close();
    this.ws = null;
  }
  reconnect() {
    if (this.closing) {
      return;
    }
    this.loggerInternal.debug(`reconnecting to ws://${this.host}:${this.port}`);
    let connection_done = false;
    const ws = new w3cwebsocket(`ws://${this.host}:${this.port}`);
    this.ws = ws;
    this.reconnectTimer = setTimeout(() => {
      if (!connection_done) {
        this.loggerInternal.warn("connection timeout");
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
          this.data.valueStore.setRecv(dataR.m, dataR.f, dataR.d);
          const target = this.member(dataR.m).value(dataR.f);
          this.data.eventEmitter.emit(eventType.valueChange(target), target);
          break;
        }
        case types.kind.text: {
          const dataR = data as types.Data<string>;
          this.data.textStore.setRecv(dataR.m, dataR.f, dataR.d);
          const target = this.member(dataR.m).text(dataR.f);
          this.data.eventEmitter.emit(eventType.textChange(target), target);
          break;
        }
        case types.kind.view: {
          const dataR = data as types.View;
          const current = this.data.viewStore.getRecv(dataR.m, dataR.f) || [];
          const diff: types.ViewComponentsDiff = {};
          for (const k of Object.keys(dataR.d)) {
            diff[k] = dataR.d[k];
          }
          mergeViewDiff(diff, dataR.l, current);
          this.data.viewStore.setRecv(dataR.m, dataR.f, current);
          const target = this.member(dataR.m).view(dataR.f);
          this.data.eventEmitter.emit(eventType.viewChange(target), target);
          break;
        }
        case types.kind.log: {
          const dataR = data as types.Log;
          for (const ll of dataR.l) {
            this.data.logStore.addRecv(dataR.m, {
              level: ll.v,
              time: new Date(ll.t),
              message: ll.m,
            });
          }
          const target = this.member(dataR.m);
          this.data.eventEmitter.emit(eventType.logChange(target), target);
          break;
        }
        case types.kind.call: {
          setTimeout(() => {
            const dataR = data as types.Call;
            const s = this.data.funcStore.dataRecv.get(
              this.data.selfMemberName
            );
            const sendResult = (res: Val | void) => {
              this.send(types.kind.callResult, {
                i: dataR.i,
                c: dataR.c,
                e: false,
                r: res === undefined ? "" : res,
              });
            };
            const sendError = (e: any) => {
              this.send(types.kind.callResult, {
                i: dataR.i,
                c: dataR.c,
                e: true,
                r: (e as Error).toString(),
              });
            };
            const sendResponse = (s: boolean) => {
              this.send(types.kind.callResponse, {
                i: dataR.i,
                c: dataR.c,
                s: s,
              });
            };
            if (s) {
              const m = s.get(dataR.f);
              if (m) {
                sendResponse(true);
                try {
                  const res = runFunc(m, dataR.a);
                  if (res instanceof Promise) {
                    res
                      .then((res: Val | void) => sendResult(res))
                      .catch((e: any) => sendError(e));
                  } else {
                    sendResult(res);
                  }
                } catch (e: any) {
                  sendError(e);
                }
              } else {
                sendResponse(false);
              }
            } else {
              sendResponse(false);
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
          const target = this.member(dataR.m);
          this.data.eventEmitter.emit(eventType.memberEntry(), target);
          break;
        }
        case types.kind.entry + types.kind.value: {
          const dataR = data as types.Entry;
          this.data.valueStore.setEntry(dataR.m, dataR.f);
          const target = this.member(dataR.m).value(dataR.f);
          this.data.eventEmitter.emit(eventType.valueEntry(target), target);
          break;
        }
        case types.kind.entry + types.kind.text: {
          const dataR = data as types.Entry;
          this.data.textStore.setEntry(dataR.m, dataR.f);
          const target = this.member(dataR.m).text(dataR.f);
          this.data.eventEmitter.emit(eventType.textEntry(target), target);
          break;
        }
        case types.kind.entry + types.kind.view: {
          const dataR = data as types.Entry;
          this.data.viewStore.setEntry(dataR.m, dataR.f);
          const target = this.member(dataR.m).view(dataR.f);
          this.data.eventEmitter.emit(eventType.viewEntry(target), target);
          break;
        }
        case types.kind.funcInfo: {
          const dataR = data as types.FuncInfo;
          this.data.funcStore.setEntry(dataR.m, dataR.f);
          this.data.funcStore.setRecv(dataR.m, dataR.f, {
            returnType: dataR.r,
            args: dataR.a.map((a) => ({
              name: a.n,
              type: a.t,
              init: a.i,
              min: a.m,
              max: a.x,
              option: a.o,
            })),
          });
          const target = this.member(dataR.m).func(dataR.f);
          this.data.eventEmitter.emit(eventType.funcEntry(target), target);
          break;
        }
      }
    };
    ws.onerror = () => {
      connection_done = true;
      this.loggerInternal.warn("connection error");
      ws.close();
    };
    ws.onclose = () => {
      connection_done = true;
      this.connected = false;
      this.loggerInternal.warn("closed");
      this.reconnectTimer = setTimeout(() => this.reconnect(), 1000);
    };
  }
  sync() {
    if (this.connected && this.ws != null) {
      if (!this.syncInit) {
        this.send(types.kind.syncInit, { m: this.data.selfMemberName });
        this.syncInit = true;
      }

      for (const [k, v] of this.data.valueStore.transferSend().entries()) {
        this.send(types.kind.value, { m: "", f: k, d: v });
      }
      for (const [k, v] of this.data.valueStore.transferReq().entries()) {
        for (const [k2, v2] of v.entries()) {
          this.send(types.kind.req + types.kind.value, { m: k, f: k2 });
        }
      }

      for (const [k, v] of this.data.textStore.transferSend().entries()) {
        this.send(types.kind.text, { m: "", f: k, d: v });
      }
      for (const [k, v] of this.data.textStore.transferReq().entries()) {
        for (const [k2, v2] of v.entries()) {
          this.send(types.kind.req + types.kind.text, { m: k, f: k2 });
        }
      }
      const viewPrev = this.data.viewStore.getSendPrev();
      for (const [k, v] of this.data.viewStore.transferSend().entries()) {
        const vPrev = viewPrev.get(k) || [];
        const diff = getViewDiff(v, vPrev);
        const diffSend: types.ViewComponentsDiff = {};
        for (const k2 of Object.keys(diff)) {
          diffSend[k2] = diff[k2];
        }
        this.send(types.kind.view, { m: "", f: k, d: diffSend });
      }
      for (const [k, v] of this.data.viewStore.transferReq().entries()) {
        for (const [k2, v2] of v.entries()) {
          this.send(types.kind.req + types.kind.view, { m: k, f: k2 });
        }
      }

      for (const [k, v] of this.data.funcStore.transferSend().entries()) {
        this.send(types.kind.funcInfo, {
          m: "",
          f: k,
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

      const logSend: types.LogLine[] = [];
      for (const l of this.data.logQueue) {
        logSend.push({ v: l.level, t: l.time.getTime(), m: l.message });
      }
      if (logSend.length > 0) {
        this.data.logQueue = [];
        this.send(types.kind.log, { m: "", l: logSend });
      }
      for (const [k, v] of this.data.logStore.transferReq().entries()) {
        this.send(types.kind.logReq, { m: k });
      }
    }
  }
  member(member: string) {
    return new Member(this, member);
  }
  members() {
    return [...this.data.valueStore.getMembers()].map((n) => this.member(n));
  }
  get membersChange() {
    return new FieldWithEvent<Member>(
      eventType.memberEntry(),
      this.data,
      "",
      ""
    );
  }
  get logAppender() {
    return {
      configure:
        (
          config: object,
          layouts: any,
          findAppender: any,
          levels?: log4jsLevels
        ) =>
        (logEvent: log4jsLoggingEvent) => {
          const ll = {
            level:
              levels != undefined
                ? log4jsLevelConvert(logEvent.level, levels)
                : 2,
            time: new Date(logEvent.startTime),
            message: util.format(...logEvent.data),
          };
          this.data.logQueue.push(ll);
        },
    };
  }
}
