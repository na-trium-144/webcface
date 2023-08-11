import { FieldBase } from "./data.js";
import { Val, AsyncFuncResult } from "./funcResult.js";
import { Member } from "./member.js";
import * as types from "./messageType.js";

export interface FuncInfo {
  returnType: number;
  args: Arg[];
  funcImpl: ((...args: Val[]) => Val) | null;
}

export class Arg {
  // todo
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
  set(func: any /* (...args: Val[]) => Val */) {
    const data = {
      // todo
      returnType: 0,
      argsType: [],
      funcImpl: data as (...args: Val[]) => Val,
    };
    if (this.data.funcStore.isSelf(this.member_)) {
      this.data.funcStore.setSend(this.member_, this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  returnType() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.returnType;
    }
    return types.argType.none_;
  }
  argsType() {
    const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
    if (funcInfo != null) {
      return funcInfo.argsType;
    }
    return [];
  }
  run(...args: Val[]) {
    const r = this.data.funcResultStore.addResult("", this);
    if (this.data.funcStore.isSelf(this.member_)) {
      const funcInfo = this.data.funcStore.getRecv(this.member_, this.field_);
      if (funcInfo != null) {
        r.resolveStarted(true);
        try {
          if (funcInfo.funcImpl != null) {
            // todo: funcImplがpromise返す場合
            r.resolveResult(funcInfo.funcImpl(...args));
          }
        } catch (e: any) {
          r.rejectResult(e);
        }
      } else {
        r.resolveStarted(false);
      }
      return r;
    } else {
      // todo
      return r;
    }
  }
}
