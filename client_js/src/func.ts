import * as types from "./messageType.js";
import { pack, unpack } from "./message.js";
import { DataStore, dataSet, dataGet } from "./data.js";

type Arg = string | number | boolean | void;
export class FuncResult {
  callerId: number;
  from: string;
  name: string;
  found = false;
  isError = false;
  errorMsg = "";
  ready = false;
  result: Arg = "";
  constructor(from: string, name: string, callerId: number) {
    this.from = from;
    this.name = name;
    this.callerId = callerId;
  }
  promise(): Promise<FuncResult> {
    return new Promise((resolve) => this.wait(resolve));
  }
  wait(resolve: (r: any) => void) {
    if (this.ready) {
      resolve(this);
    } else {
      setTimeout(() => this.wait(resolve));
    }
  }
}

export interface FuncInfoInternal {
  returnType: number;
  argsType: number[];
  funcImpl: ((...args: Arg[]) => Arg) | null;
}

export class Func {
  funcs: DataStore<FuncInfoInternal>;
  results: FuncResult[];
  send: (m: ArrayBuffer) => void;
  from: string;
  name: string;
  constructor(
    send: (m: ArrayBuffer) => void,
    funcs: DataStore<FuncInfoInternal>,
    results: FuncResult[],
    from: string,
    name: string
  ) {
    this.send = send;
    this.funcs = funcs;
    this.results = results;
    this.from = from;
    this.name = name;
  }
  set(data: any /* (...args: Arg[]) => Arg */) {
    dataSet(this.funcs, this.from, this.name, {
      // todo
      returnType: 0,
      argsType: [],
      funcImpl: data as (...args: Arg[]) => Arg,
    });
  }
  addResult() {
    const r = new FuncResult(this.from, this.name, this.results.length);
    this.results.push(r);
    return r;
  }
  returnType() {
    const funcInfo = dataGet(this.funcs, this.from, this.name);
    if (funcInfo != null) {
      return funcInfo.returnType;
    }
    return types.argType.none_;
  }
  argsType() {
    const funcInfo = dataGet(this.funcs, this.from, this.name);
    if (funcInfo != null) {
      return funcInfo.argsType;
    }
    return [];
  }
  run(...args: Arg[]) {
    if (this.from === "") {
      const funcInfo = dataGet(this.funcs, this.from, this.name);
      const r = this.addResult();
      if (funcInfo != null) {
        r.found = true;
        try {
          if (funcInfo.funcImpl != null) {
            r.result = funcInfo.funcImpl(...args);
          }
        } catch (e: any) {
          r.errorMsg = (e as Error).toString();
          r.isError = true;
        }
      } else {
        r.found = false;
      }
      r.ready = true;
      return r.promise();
    } else {
      const r = this.addResult();
      this.send(
        pack(types.kind.call, {
          i: r.callerId,
          c: "",
          r: this.from,
          n: this.name,
          a: args,
        } as types.Call)
      );
      return r.promise();
    }
  }
}
