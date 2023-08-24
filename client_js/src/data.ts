import {
  FieldBase,
  AsyncFuncResult,
  FieldBaseWithEvent,
  eventType,
} from "./clientData.js";
import { Val, FuncInfo, Arg } from "./funcInfo.js";
import { argType } from "./message.js";

export class Member extends FieldBase {
  constructor(base: FieldBase, member = "") {
    super(base.data, member || base.member_, "");
  }
  get name() {
    return this.member_;
  }
  value(name: string) {
    return new Value(this, name);
  }
  text(name: string) {
    return new Text(this, name);
  }
  func(name: string) {
    return new Func(this, name);
  }
  logs() {
    return new Logs(this);
  }
  values() {
    return this.data.valueStore
      .getEntry(this.member_)
      .map((n) => this.value(n));
  }
  texts() {
    return this.data.textStore.getEntry(this.member_).map((n) => this.text(n));
  }
  funcs() {
    return this.data.funcStore.getEntry(this.member_).map((n) => this.func(n));
  }
  get valuesChange() {
    return new FieldBaseWithEvent<Value>(
      eventType.valueEntry(this),
      this.data,
      this.member_
    );
  }
  get textsChange() {
    return new FieldBaseWithEvent<Text>(
      eventType.textEntry(this),
      this.data,
      this.member_
    );
  }
  get funcsChange() {
    return new FieldBaseWithEvent<Func>(
      eventType.funcEntry(this),
      this.data,
      this.member_
    );
  }
}

export class Value extends FieldBaseWithEvent<Value> {
  constructor(base: FieldBase, field = "") {
    super("", base.data, base.member_, field || base.field_, () =>
      this.tryGet()
    );
    this.eventType_ = eventType.valueChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.data.valueStore.getRecv(this.member_, this.field_);
  }
  get() {
    const v = this.tryGet();
    if (v === null) {
      return 0;
    } else {
      return v;
    }
  }
  set(data: number) {
    if (this.data.valueStore.isSelf(this.member_)) {
      this.data.valueStore.setSend(this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
}
export class Text extends FieldBaseWithEvent<Text> {
  constructor(base: FieldBase, field = "") {
    super("", base.data, base.member_, field || base.field_, () =>
      this.tryGet()
    );
    this.eventType_ = eventType.textChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.data.textStore.getRecv(this.member_, this.field_);
  }
  get() {
    const v = this.tryGet();
    if (v === null) {
      return "";
    } else {
      return v;
    }
  }
  set(data: string) {
    if (this.data.textStore.isSelf(this.member_)) {
      this.data.textStore.setSend(this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
}
export class Logs extends FieldBaseWithEvent<Logs> {
  constructor(base: FieldBase) {
    super("", base.data, base.member_, "", () => this.tryGet());
    this.eventType_ = eventType.logChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.data.logStore.getRecv(this.member_);
  }
  get() {
    const v = this.tryGet();
    if (v === null) {
      return [];
    } else {
      return v;
    }
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
    if (funcInfo != null) {
      r.resolveStarted(true);
      try {
        if (funcInfo.funcImpl != undefined) {
          // funcImplがpromise返す場合もそのままresolveにぶちこめばよいはず
          let res: Val | Promise<Val> | void = runFunc(funcInfo, args);
          if (res === undefined) {
            res = "";
          }
          r.resolveResult(res);
        } else {
          this.data.callFunc(r, this, args);
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
}
