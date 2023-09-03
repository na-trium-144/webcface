import { Member } from "./member.js";
import { argType } from "./message.js";
import { Field, FieldBase } from "./field.js";

export type Val = string | number | boolean;

export interface FuncInfo {
  returnType: number;
  args: Arg[];
  funcImpl?: (...args: Val[]) => Val | Promise<Val> | void;
  // call
}

export interface Arg {
  name?: string;
  type?: number;
  init?: Val | null;
  min?: number | null;
  max?: number | null;
  option?: string[] | number[];
}

export class FuncNotFoundError extends Error {
  constructor(base: FieldBase) {
    super(`member("${base.member_}").func("${base.field_}") is not set`);
    this.name = "FuncNotFoundError";
  }
}

export class AsyncFuncResult extends FieldBase {
  callerId: number;
  caller: string;
  // 関数が開始したらtrue, 存在しなければfalse
  // falseの場合rejectResultも自動で呼ばれる
  resolveStarted: (r: boolean) => void = () => undefined;
  // 結果をセットする
  resolveResult: (r: Val | Promise<Val>) => void = () => undefined;
  // 例外をセットする
  rejectResult: (e: any) => void = () => undefined;
  started: Promise<boolean>;
  result: Promise<Val>;
  constructor(callerId: number, caller: string, base: FieldBase) {
    super(base.member_, base.field_);
    this.callerId = callerId;
    this.caller = caller;
    this.started = new Promise((res) => {
      this.resolveStarted = (r: boolean) => {
        res(r);
        if (!r) {
          this.rejectResult(new FuncNotFoundError(this));
        }
      };
    });
    this.result = new Promise((res, rej) => {
      this.resolveResult = res;
      this.rejectResult = rej;
    });
  }
}

export function runFunc(fi: FuncInfo, args: Val[]) {
  if (fi.args.length === args.length) {
    const newArgs: Val[] = args.map((a, i) => {
      switch (fi.args[i].type) {
        case argType.string_:
          return String(a);
        case argType.boolean_:
          if (typeof a === "string") {
            return a !== "";
          } else {
            return !!a;
          }
        case argType.int_:
          return parseInt(String(a));
        case argType.float_:
          return parseFloat(String(a));
        default:
          return a;
      }
    });
    if (fi.funcImpl != undefined) {
      return fi.funcImpl(...newArgs);
    }
    return undefined;
  } else {
    throw new Error(
      `require ${fi.args.length} arguments, but got ${args.length}`
    );
  }
}

export class Func extends Field {
  constructor(base: Field, field = "") {
    super(base.data, base.member_, field || base.field_);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  set(
    func: (...args: any[]) => Val | Promise<Val> | void,
    returnType: number = argType.none_,
    args: Arg[] = []
  ) {
    const data: FuncInfo = {
      returnType: returnType,
      args: args,
      funcImpl: func,
    };
    if (this.data.funcStore.isSelf(this.member_)) {
      this.data.funcStore.setSend(this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  get returnType() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.returnType;
    }
    return argType.none_;
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
    if (this.data.isSelf(this.member_)) {
      if (funcInfo != null && funcInfo.funcImpl != undefined) {
        r.resolveStarted(true);
        try {
          // funcImplがpromise返す場合もそのままresolveにぶちこめばよいはず
          let res: Val | Promise<Val> | void = runFunc(funcInfo, args);
          if (res === undefined) {
            res = "";
          }
          r.resolveResult(res);
        } catch (e: any) {
          r.rejectResult(e);
        }
      } else {
        r.resolveStarted(false);
      }
    } else {
      this.data.callFunc(r, this, args);
    }
  }
  runAsync(...args: Val[]) {
    const r = this.data.funcResultStore.addResult("", this);
    setTimeout(() => {
      this.runImpl(r, args);
    });
    return r;
  }
}
