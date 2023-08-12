import { AsyncFuncResult } from "./funcResult.js";
import { FuncInfo } from "./func.js";

export class ClientData {
  selfMemberName: string;
  valueStore: SyncDataStore<number>;
  textStore: SyncDataStore<string>;
  funcStore: SyncDataStore<FuncInfo>;
  funcResultStore: FuncResultStore;
  constructor(name: string) {
    this.selfMemberName = name;
    this.valueStore = new SyncDataStore<number>(name);
    this.textStore = new SyncDataStore<string>(name);
    this.funcStore = new SyncDataStore<FuncInfo>(name);
    this.funcResultStore = new FuncResultStore();
  }
}

class SyncDataStore<T> {
  dataSend: Map<string, T>;
  dataRecv: Map<string, Map<string, T>>;
  entry: Map<string, string[]>;
  req: Map<string, Map<string, boolean>>;
  reqSend: Map<string, Map<string, boolean>>;
  selfMemberName: string;
  constructor(name: string) {
    this.selfMemberName = name;
    this.dataSend = new Map();
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
      this.dataRecv.set(member, [[field, data]]);
    }
  }
  //! data_recvからデータを返す or なければreq,req_sendをtrueにセット
  getRecv(member: string, field: string) {
    if (
      !this.isSelf(member) &&
      (!this.req.has(member) ||
        !this.req.get(member).has(field) ||
        this.req.get(member).get(field) === false)
    ) {
      const m = this.req.get(member);
      if (m) {
        m.set(field, true);
      } else {
        this.req.set(member, [[field, true]]);
      }
      const m2 = this.reqSend.get(member);
      if (m2) {
        m2.set(field, true);
      } else {
        this.reqSend.set(member, [[field, true]]);
      }
    }
    if (this.dataRecv.has(member) && this.dataRecv.get(member).has(field)) {
      return this.dataRecv.get(member).get(field) as T;
    } else {
      return null;
    }
  }
  //! data_recvからデータを削除, req,req_sendをfalseにする
  unsetRecv(member: string, field: string) {
    if (
      !this.isSelf(member) &&
      this.req.has(member) &&
      this.req.get(member).has(field) &&
      this.req.get(member).get(field) === true
    ) {
      this.req.get(member).set(field, false);
      const m2 = this.reqSend.get(member);
      if (m2) {
        m2.set(field, false);
      } else {
        this.reqSend.set(member, [[field, false]]);
      }
    }
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
    this.dataSend = new Map();
    return s;
  }
  //! req_sendを返し、req_sendをクリア
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
