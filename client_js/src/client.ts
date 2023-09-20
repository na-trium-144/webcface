import * as Message from "./message.js";
import { ClientData } from "./clientData.js";
import { Member } from "./member.js";
import { AsyncFuncResult, runFunc, Val } from "./func.js";
import {
  log4jsLoggingEvent,
  log4jsLevels,
  log4jsLevelConvert,
  LogLine,
} from "./logger.js";
import { getViewDiff, mergeViewDiff } from "./view.js";
import websocket from "websocket";
const w3cwebsocket = websocket.w3cwebsocket;
import util from "util";
import { getLogger } from "@log4js-node/log4js-api";
import { Field, FieldBase } from "./field.js";
import { FieldWithEvent, eventType } from "./event.js";

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
  send(msg: Message.AnyMessage[]) {
    this.ws?.send(Message.pack(msg));
  }
  callFunc(r: AsyncFuncResult, b: FieldBase, args: Val[]) {
    this.send([
      {
        kind: Message.kind.call,
        i: r.callerId,
        c: this.data.getMemberIdFromName(r.caller),
        r: this.data.getMemberIdFromName(b.member_),
        f: b.field_,
        a: args,
      },
    ]);
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
      const messages = Message.unpack(event.data as ArrayBuffer);
      for (const data of messages) {
        switch (data.kind) {
          case Message.kind.sync: {
            const dataR = data as Message.Sync;
            const member = this.data.getMemberNameFromId(dataR.m);
            this.data.syncTimeStore.setRecv(member, new Date(dataR.t));
            const target = this.member(member);
            this.data.eventEmitter.emit(eventType.sync(target), target);
            break;
          }
          case Message.kind.valueRes: {
            const dataR = data as Message.ValueRes;
            const [member, field] = this.data.valueStore.getReq(
              dataR.i,
              dataR.f
            );
            this.data.valueStore.setRecv(member, field, dataR.d);
            const target = this.member(member).value(field);
            this.data.eventEmitter.emit(eventType.valueChange(target), target);
            break;
          }
          case Message.kind.textRes: {
            const dataR = data as Message.TextRes;
            const [member, field] = this.data.textStore.getReq(
              dataR.i,
              dataR.f
            );
            this.data.textStore.setRecv(member, field, dataR.d);
            const target = this.member(member).text(field);
            this.data.eventEmitter.emit(eventType.textChange(target), target);
            break;
          }
          case Message.kind.viewRes: {
            const dataR = data as Message.ViewRes;
            const [member, field] = this.data.viewStore.getReq(
              dataR.i,
              dataR.f
            );
            const current = this.data.viewStore.getRecv(member, field) || [];
            const diff: Message.ViewComponentsDiff = {};
            for (const k of Object.keys(dataR.d)) {
              diff[k] = dataR.d[k];
            }
            mergeViewDiff(diff, dataR.l, current);
            this.data.viewStore.setRecv(member, field, current);
            const target = this.member(member).view(field);
            this.data.eventEmitter.emit(eventType.viewChange(target), target);
            break;
          }
          case Message.kind.log: {
            const dataR = data as Message.Log;
            const member = this.data.getMemberNameFromId(dataR.m);
            const log = this.data.logStore.getRecv(member) || [];
            const target = this.member(member).log();
            for (const ll of dataR.l) {
              const ll2: LogLine = {
                level: ll.v,
                time: new Date(ll.t),
                message: ll.m,
              };
              log.push(ll2);
            }
            this.data.logStore.setRecv(member, log);
            this.data.eventEmitter.emit(eventType.logAppend(target), target);
            break;
          }
          case Message.kind.call: {
            setTimeout(() => {
              const dataR = data as Message.Call;
              const s = this.data.funcStore.dataRecv.get(
                this.data.selfMemberName
              );
              const sendResult = (res: Val | void) => {
                this.send([
                  {
                    kind: Message.kind.callResult,
                    i: dataR.i,
                    c: dataR.c,
                    e: false,
                    r: res === undefined ? "" : res,
                  },
                ]);
              };
              const sendError = (e: any) => {
                this.send([
                  {
                    kind: Message.kind.callResult,
                    i: dataR.i,
                    c: dataR.c,
                    e: true,
                    r: (e as Error).toString(),
                  },
                ]);
              };
              const sendResponse = (s: boolean) => {
                this.send([
                  {
                    kind: Message.kind.callResponse,
                    i: dataR.i,
                    c: dataR.c,
                    s: s,
                  },
                ]);
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
          case Message.kind.callResponse: {
            const dataR = data as Message.CallResponse;
            const r = this.data.funcResultStore.getResult(dataR.i);
            if (r != undefined) {
              r.resolveStarted(dataR.s);
            } else {
              this.loggerInternal.error(
                `error receiving call result id=${dataR.i}`
              );
            }
            break;
          }
          case Message.kind.callResult: {
            const dataR = data as Message.CallResult;
            const r = this.data.funcResultStore.getResult(dataR.i);
            if (r != undefined) {
              if (dataR.e) {
                r.rejectResult(new Error(String(dataR.r)));
              } else {
                r.resolveResult(dataR.r);
              }
            } else {
              this.loggerInternal.error(
                `error receiving call result id=${dataR.i}`
              );
            }
            break;
          }
          case Message.kind.syncInit: {
            const dataR = data as Message.SyncInit;
            this.data.valueStore.addMember(dataR.M);
            this.data.textStore.addMember(dataR.M);
            this.data.funcStore.addMember(dataR.M);
            this.data.memberIds.set(dataR.M, dataR.m);
            const target = this.member(dataR.M);
            this.data.eventEmitter.emit(eventType.memberEntry(), target);
            break;
          }
          case Message.kind.valueEntry: {
            const dataR = data as Message.Entry;
            const member = this.data.getMemberNameFromId(dataR.m);
            this.data.valueStore.setEntry(member, dataR.f);
            const target = this.member(member).value(dataR.f);
            this.data.eventEmitter.emit(eventType.valueEntry(target), target);
            break;
          }
          case Message.kind.textEntry: {
            const dataR = data as Message.Entry;
            const member = this.data.getMemberNameFromId(dataR.m);
            this.data.textStore.setEntry(member, dataR.f);
            const target = this.member(member).text(dataR.f);
            this.data.eventEmitter.emit(eventType.textEntry(target), target);
            break;
          }
          case Message.kind.viewEntry: {
            const dataR = data as Message.Entry;
            const member = this.data.getMemberNameFromId(dataR.m);
            this.data.viewStore.setEntry(member, dataR.f);
            const target = this.member(member).view(dataR.f);
            this.data.eventEmitter.emit(eventType.viewEntry(target), target);
            break;
          }
          case Message.kind.funcInfo: {
            const dataR = data as Message.FuncInfo;
            const member = this.data.getMemberNameFromId(dataR.m);
            this.data.funcStore.setEntry(member, dataR.f);
            this.data.funcStore.setRecv(member, dataR.f, {
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
            const target = this.member(member).func(dataR.f);
            this.data.eventEmitter.emit(eventType.funcEntry(target), target);
            break;
          }
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
      const msg: Message.AnyMessage[] = [];
      let isFirst = false;
      if (!this.syncInit) {
        msg.push({
          kind: Message.kind.syncInit,
          M: this.data.selfMemberName,
          m: 0,
        });
        this.syncInit = true;
        isFirst = true;
      }

      msg.push({ kind: Message.kind.sync, m: 0, t: new Date().getTime() });

      for (const [k, v] of this.data.valueStore
        .transferSend(isFirst)
        .entries()) {
        msg.push({ kind: Message.kind.value, f: k, d: v });
      }
      for (const [k, v] of this.data.valueStore
        .transferReq(isFirst)
        .entries()) {
        for (const [k2, v2] of v.entries()) {
          msg.push({ kind: Message.kind.valueReq, M: k, f: k2, i: v2 });
        }
      }

      for (const [k, v] of this.data.textStore
        .transferSend(isFirst)
        .entries()) {
        msg.push({ kind: Message.kind.text, f: k, d: v });
      }
      for (const [k, v] of this.data.textStore.transferReq(isFirst).entries()) {
        for (const [k2, v2] of v.entries()) {
          msg.push({ kind: Message.kind.textReq, M: k, f: k2, i: v2 });
        }
      }
      const viewPrev = this.data.viewStore.getSendPrev(isFirst);
      for (const [k, v] of this.data.viewStore
        .transferSend(isFirst)
        .entries()) {
        const vPrev = viewPrev.get(k) || [];
        const diff = getViewDiff(v, vPrev);
        msg.push({ kind: Message.kind.view, f: k, d: diff, l: v.length });
      }
      for (const [k, v] of this.data.viewStore.transferReq(isFirst).entries()) {
        for (const [k2, v2] of v.entries()) {
          msg.push({ kind: Message.kind.viewReq, M: k, f: k2, i: v2 });
        }
      }

      for (const [k, v] of this.data.funcStore
        .transferSend(isFirst)
        .entries()) {
        if (!this.data.funcStore.isHidden(k)) {
          msg.push({
            kind: Message.kind.funcInfo,
            f: k,
            r: v.returnType,
            a: v.args.map((a) => ({
              n: a.name || "",
              t: a.type != undefined ? a.type : Message.valType.none_,
              i: a.init != undefined ? a.init : null,
              m: a.min != undefined ? a.min : null,
              x: a.max != undefined ? a.max : null,
              o: a.option != undefined ? a.option : [],
            })),
          });
        }
      }

      const logSend: Message.LogLine[] = [];
      if (this.data.logStore.getRecv(this.name) == null) {
        this.data.logStore.setRecv(this.name, []);
      }
      const logRecv = this.data.logStore.getRecv(this.name) as LogLine[];
      for (const l of this.data.logQueue) {
        logSend.push({ v: l.level, t: l.time.getTime(), m: l.message });
        logRecv.push(l);
      }
      if (logSend.length > 0) {
        this.data.logQueue = [];
        msg.push({ kind: Message.kind.log, l: logSend });
      }
      for (const [k, v] of this.data.logStore.transferReq(isFirst).entries()) {
        msg.push({ kind: Message.kind.logReq, M: k });
      }

      this.send(msg);
    }
  }
  member(member: string) {
    return new Member(this, member);
  }
  members() {
    return [...this.data.valueStore.getMembers()].map((n) => this.member(n));
  }
  get onMemberEntry() {
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
