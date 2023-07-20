import * as types from "./messageType.js";
import { pack, unpack } from "./message.js";

export class FuncResult {
  callerId: number;
  from: string;
  name: string;
  found = false;
  isError = false;
  errorMsg = "";
  ready = false;
  result = "";
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

export interface FuncStore {
  name: string;
  func: (...args: string[]) => any;
}

export class Func {
  funcs: FuncStore[];
  results: FuncResult[];
  send: (m: ArrayBuffer) => void;
  from: string;
  name: string;
  constructor(
    send: (m: ArrayBuffer) => void,
    funcs: FuncStore[],
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
  set(func: (...args: string[]) => any) {
    // todo:引数の扱い
    const s = this.funcs.find((s) => s.name === this.name);
    if (s) {
      s.func = func;
    } else {
      this.funcs.push({ name: this.name, func: func });
    }
  }
  addResult() {
    const r = new FuncResult(this.from, this.name, this.results.length);
    this.results.push(r);
    return r;
  }
  run(...args: string[]) {
    if (this.from === "") {
      const r = this.addResult();
      const s = this.funcs.find((s) => s.name === this.name);
      if (s) {
        r.found = true;
        try {
          r.result = String(s.func(...args));
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
