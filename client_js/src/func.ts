import { FieldBase } from "./data.js";
import { Val, AsyncFuncResult } from "./funcResult.js";
import { Member } from "./member.js";
import * as types from "./messageType.js";

export interface FuncInfo {
  returnType: number;
  args: Arg[];
  funcImpl?: (...args: Val[]) => Val | Promise<Val>;
  call?: (r: AsyncFuncResult, args: Val[]) => void;
}
export function runFunc(fi: FuncInfo, args: Val[]) {
  if (fi.args.length === args.length) {
    const newArgs: Val[] = args.map((a, i) => {
      switch (fi.args[i].type) {
        case types.argType.string_:
          return String(a);
        case types.argType.boolean_:
          if (typeof a === "string") {
            return a !== "";
          } else {
            return !!a;
          }
        case types.argType.int_:
          return parseInt(String(a));
        case types.argType.float_:
          return parseFloat(String(a));
        default:
          return a;
      }
    });
    let res: Val | Promise<Val> = fi.funcImpl(...newArgs);
    if (res instanceof Promise) {
      res = await res;
    }
    return res;
  } else {
    throw new Error(
      `require ${fi.args.length} arguments, but got ${args.length}`
    );
  }
}

export interface Arg {
  name?: string;
  type?: number;
  init?: string | number | boolean | null;
  min?: number | null;
  max?: number | null;
  option?: string[] | number[];
}

export class Func extends FieldBase {
  constructor(base: FieldBase, field = "") {
    super(base.data, base.member_, field || base.field_);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  set(
    func: (...args: any[]) => Val | Promise<Val>,
    returnType: number = types.argType.none_,
    args: Args[] = []
  ) {
    const data = {
      // todo
      returnType: returnType,
      args: args,
      funcImpl: data as (...args: Val[]) => Val,
    };
    if (this.data.funcStore.isSelf(this.member_)) {
      this.data.funcStore.setSend(this.member_, this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  get returnType() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.returnType;
    }
    return types.argType.none_;
  }
  get args() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.args;
    }
    return [];
  }
  runImpl(r: AsyncFuncResult, args: Val[]) {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      r.resolveStarted(true);
      try {
        if (funcInfo.funcImpl != undefined) {
          // funcImplがpromise返す場合もそのままresolveにぶちこめばよいはず
          r.resolveResult(runFunc(funcInfo, args));
        } else if (funcInfo.call != undefined) {
          funcInfo.call(r, args);
        }
      } catch (e: any) {
        r.rejectResult(e);
      }
    } else {
      r.resolveStarted(false);
    }
  }
  runAsync(...args: Val[]) {
    const r = this.data.funcResultStore.addResult("", this);
    setTimeout(() => {
      this.runImpl(r, args);
    });
    return r;
  }
  run(...args: Val[]) {
    const r = this.data.funcResultStore.addResult("", this);
    this.runImpl(r, args);
    return r;
  }
}
