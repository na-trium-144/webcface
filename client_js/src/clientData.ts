import { Val, FuncInfo, AsyncFuncResult } from "./func.js";
import { EventEmitter } from "eventemitter3";
import { LogLine } from "./logger.js";
import * as Message from "./message.js";
import { FieldBase } from "./field.js";

export class ClientData {
  selfMemberName: string;
  valueStore: SyncDataStore<number>;
  textStore: SyncDataStore<string>;
  funcStore: SyncDataStore<FuncInfo>;
  viewStore: SyncDataStore<Message.ViewComponent[]>;
  logStore: LogStore;
  funcResultStore: FuncResultStore;
  callFunc: (r: AsyncFuncResult, b: FieldBase, args: Val[]) => void;
  eventEmitter: EventEmitter;
  logQueue: LogLine[];
  constructor(
    name: string,
    callFunc: (r: AsyncFuncResult, b: FieldBase, args: Val[]) => void
  ) {
    this.selfMemberName = name;
    this.valueStore = new SyncDataStore<number>(name);
    this.textStore = new SyncDataStore<string>(name);
    this.funcStore = new SyncDataStore<FuncInfo>(name);
    this.viewStore = new SyncDataStore<Message.ViewComponent[]>(name);
    this.logStore = new LogStore(name);
    this.funcResultStore = new FuncResultStore();
    this.callFunc = callFunc;
    this.eventEmitter = new EventEmitter();
    this.logQueue = [];
  }
  isSelf(member: string) {
    return this.selfMemberName === member;
  }
}

class SyncDataStore<T> {
  dataSend: Map<string, T>;
  dataSendPrev: Map<string, T>;
  dataRecv: Map<string, Map<string, T>>;
  entry: Map<string, string[]>;
  req: Map<string, Map<string, boolean>>;
  reqSend: Map<string, Map<string, boolean>>;
  selfMemberName: string;
  constructor(name: string) {
    this.selfMemberName = name;
    this.dataSend = new Map();
    this.dataSendPrev = new Map();
    this.dataRecv = new Map();
    this.entry = new Map();
    this.req = new Map();
    this.reqSend = new Map();
  }
  isSelf(member: string) {
    return this.selfMemberName === member;
  }
  //! 送信するデータをdata_sendとdata_recv[self_member_name]にセット
  setSend(field: string, data: T) {
    this.dataSend.set(field, data);
    this.setRecv(this.selfMemberName, field, data);
  }
  //! 受信したデータをdata_recvにセット
  setRecv(member: string, field: string, data: T) {
    const m = this.dataRecv.get(member);
    if (m) {
      m.set(field, data);
    } else {
      this.dataRecv.set(member, new Map([[field, data]]));
    }
  }
  //! data_recvからデータを返す or なければreq,req_sendをtrueにセット
  getRecv(member: string, field: string) {
    if (!this.isSelf(member) && this.req.get(member)?.get(field) !== true) {
      const m = this.req.get(member);
      if (m) {
        m.set(field, true);
      } else {
        this.req.set(member, new Map([[field, true]]));
      }
      const m2 = this.reqSend.get(member);
      if (m2) {
        m2.set(field, true);
      } else {
        this.reqSend.set(member, new Map([[field, true]]));
      }
    }
    const m = this.dataRecv.get(member)?.get(field);
    if (m != undefined) {
      return m;
    }
    return null;
  }
  //! data_recvからデータを削除, req,req_sendをfalseにする
  unsetRecv(member: string, field: string) {
    if (!this.isSelf(member) && this.req.get(member)?.get(field) === true) {
      this.req.get(member)?.set(field, false);
      const m2 = this.reqSend.get(member);
      if (m2) {
        m2.set(field, false);
      } else {
        this.reqSend.set(member, new Map([[field, false]]));
      }
    }
    this.dataRecv.get(member)?.delete(field);
  }
  //! member名のりすとを取得(entryから)
  getMembers() {
    return this.entry.keys();
  }
  //! entryを取得
  getEntry(member: string) {
    return this.entry.get(member) || [];
  }
  //! entryにmember名のみ追加
  addMember(member: string) {
    this.entry.set(member, []);
  }
  //! 受信したentryを追加
  setEntry(member: string, e: string) {
    this.entry.set(member, (this.getEntry(member) || []).concat([e]));
  }
  //! data_sendを返し、data_sendをクリア
  transferSend() {
    const s = this.dataSend;
    this.dataSendPrev = s;
    this.dataSend = new Map();
    return s;
  }
  getSendPrev() {
    return this.dataSendPrev;
  }
  //! req_sendを返し、req_sendをクリア
  transferReq() {
    const r = this.reqSend;
    this.reqSend = new Map();
    return r;
  }
}

class LogStore {
  dataRecv: Map<string, LogLine[]>;
  req: Map<string, boolean>;
  reqSend: Map<string, boolean>;
  selfMemberName: string;
  constructor(name: string) {
    this.selfMemberName = name;
    this.dataRecv = new Map();
    this.req = new Map();
    this.reqSend = new Map();
  }
  isSelf(member: string) {
    return this.selfMemberName === member;
  }
  addRecv(member: string, log: LogLine) {
    const m = this.dataRecv.get(member);
    if (m) {
      m.push(log);
    } else {
      this.dataRecv.set(member, [log]);
    }
  }
  getRecv(member: string) {
    if (!this.isSelf(member) && this.req.get(member) !== true) {
      this.req.set(member, true);
      this.reqSend.set(member, true);
    }
    const m = this.dataRecv.get(member);
    if (m != undefined) {
      return m;
    }
    return null;
  }
  transferReq() {
    const r = this.reqSend;
    this.reqSend = new Map();
    return r;
  }
}
class FuncResultStore {
  results: AsyncFuncResult[] = [];
  addResult(caller: string, base: FieldBase) {
    const callerId = this.results.length;
    this.results.push(new AsyncFuncResult(callerId, caller, base));
    return this.results[callerId];
  }
  getResult(callerId: number) {
    return this.results[callerId];
  }
}
