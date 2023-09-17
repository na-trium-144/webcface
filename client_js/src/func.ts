import { Member } from "./member.js";
import { valType } from "./message.js";
import { Field, FieldBase } from "./field.js";

export type Val = string | number | boolean;

export interface FuncInfo {
  returnType: number;
  args: Arg[];
  funcImpl?: FuncCallback;
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

export class AsyncFuncResult extends Field {
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
  constructor(callerId: number, caller: string, base: Field) {
    super(base.data, base.member_, base.field_);
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
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
}

export function runFunc(fi: FuncInfo, args: Val[]) {
  if (fi.args.length === args.length) {
    const newArgs: Val[] = args.map((a, i) => {
      switch (fi.args[i].type) {
        case valType.string_:
          return String(a);
        case valType.boolean_:
          if (typeof a === "string") {
            return a !== "";
          } else {
            return !!a;
          }
        case valType.int_:
          return parseInt(String(a));
        case valType.float_:
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

export type FuncCallback = (...args: any[]) => Val | Promise<Val> | void;
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
  setInfo(data: FuncInfo) {
    if (this.data.funcStore.isSelf(this.member_)) {
      this.data.funcStore.setSend(this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  set(
    func: FuncCallback,
    returnType: number = valType.none_,
    args: Arg[] = []
  ) {
    this.setInfo({
      returnType: returnType,
      args: args,
      funcImpl: func,
    });
  }
  get returnType() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.returnType;
    }
    return valType.none_;
  }
  get args() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.args.map((a) => ({ ...a }));
    }
    return [];
  }
  set hidden(h: boolean) {
    if (this.data.funcStore.isSelf(this.member_)) {
      this.data.funcStore.setHidden(this.field_, h);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  free() {
    this.data.funcStore.unsetRecv(this.member_, this.field_);
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
export class AnonymousFunc {
  static fieldId = 0;
  static fieldNameTmp() {
    return `.tmp${++this.fieldId}`;
  }

  base_: Func | null;
  func_: FuncCallback;
  returnType_: number;
  args_: Arg[];
  constructor(
    base: Field | null,
    func: FuncCallback,
    returnType: number,
    args: Arg[]
  ) {
    this.func_ = func;
    this.returnType_ = returnType;
    this.args_ = args;
    if (base == null) {
      this.base_ = null;
    } else {
      this.base_ = new Func(base, AnonymousFunc.fieldNameTmp());
      this.base_.set(func, returnType, args);
      this.base_.hidden = true;
    }
  }
  lockTo(target: Func) {
    if (this.base_ == null) {
      this.base_ = new Func(target, AnonymousFunc.fieldNameTmp());
      this.base_.set(this.func_, this.returnType_, this.args_);
      this.base_.hidden = true;
    }
    const fi = this.base_.data.funcStore.getRecv(
      this.base_.member_,
      this.base_.field_
    );
    if (fi) {
      target.setInfo(fi);
      this.base_.free();
    } else {
      // コンストラクタかlockToのどちらかで必ずsetされているはずなのであり得ないが
      throw new Error("Error in AnosymousFunc.lockTo()");
    }
  }
}
