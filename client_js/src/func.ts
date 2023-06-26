import * as types from "./messageType";
import { pack, unpack } from "./message";
import * as types from "./messageType";

export class FuncResult {
  from: string;
  name: string;
  found: boolean;
  is_error = false;
  error_msg: string;
  ready: boolean;
  result: string;
  caller_id: number;
  wait(resolve: (r) => void) {
    if (ready) {
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
  send: (m: ArrayBuffer) => void;
  from: string;
  name: string;
  constructor(
    send: (m: ArrayBuffer) => void,
    funcs: FuncStore[],
    from: string,
    name: string
  ) {
    this.send = send;
    this.funcs = funcs;
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
  run(...args: string[]) {
    if (from === "") {
      const r = new FuncResult();
      r.from = "";
      r.name = this.name;
      const s = this.funcs.find((s) => s.n === this.name);
      if (s) {
        r.found = true;
        try {
          r.result = String(s.func(...args));
        } catch (e: Error) {
          r.error_msg = e.toString();
          r.is_error = true;
        }
      } else {
        r.found = false;
      }
      r.ready = true;
      return new Promise((resolve) => resolve(r));
    } else {
      const r = new FuncResult();
      r.from = this.from;
      r.name = this.name;
      r.caller_id = 0;
      this.send(
        pack(types.kind.call, {
          i: r.caller_id,
          c: "",
          f: this.from,
          n: this.name,
          a: args,
        })
      );
      return new Promise((resolve) => r.wait(resolve));
    }
  }
}
