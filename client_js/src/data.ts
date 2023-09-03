import {
  FieldBase,
  Field,
  AsyncFuncResult,
  FieldWithEvent,
  eventType,
  ClientData,
} from "./clientData.js";
import { Val, FuncInfo, Arg } from "./funcInfo.js";
import { argType } from "./message.js";
import * as types from "./message.js";
import { viewComponentTypes, viewColor } from "./view.js";

export class Member extends Field {
  constructor(base: Field, member = "") {
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
  view(name: string) {
    return new View(this, name);
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
  views() {
    return this.data.viewStore.getEntry(this.member_).map((n) => this.view(n));
  }
  funcs() {
    return this.data.funcStore.getEntry(this.member_).map((n) => this.func(n));
  }
  get valuesChange() {
    return new FieldWithEvent<Value>(
      eventType.valueEntry(this),
      this.data,
      this.member_
    );
  }
  get textsChange() {
    return new FieldWithEvent<Text>(
      eventType.textEntry(this),
      this.data,
      this.member_
    );
  }
  get funcsChange() {
    return new FieldWithEvent<Func>(
      eventType.funcEntry(this),
      this.data,
      this.member_
    );
  }
  get viewsChange() {
    return new FieldWithEvent<View>(
      eventType.viewEntry(this),
      this.data,
      this.member_
    );
  }
}

export class Value extends FieldWithEvent<Value> {
  constructor(base: Field, field = "") {
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
export class Text extends FieldWithEvent<Text> {
  constructor(base: Field, field = "") {
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

export class View extends FieldWithEvent<View> {
  constructor(base: Field, field = "") {
    super("", base.data, base.member_, field || base.field_, () =>
      this.tryGet()
    );
    this.eventType_ = eventType.viewChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.data.viewStore.getRecv(this.member_, this.field_);
  }
  get() {
    const v = this.tryGet();
    if (v === null) {
      return [];
    } else {
      return v.map((v) => new ViewComponent(v, this.data));
    }
  }
  set(data: ViewComponent[]) {
    if (this.data.viewStore.isSelf(this.member_)) {
      this.data.viewStore.setSend(
        this.field_,
        data.map((v) => v.toMessage())
      );
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
}
export class Logs extends FieldWithEvent<Logs> {
  constructor(base: Field) {
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

export const viewComponents = {
  newLine: () => new ViewComponent(viewComponentTypes.newLine),
  text: (t: string) => new ViewComponent(t),
  button: (t: string, f: Func) => {
    const v = new ViewComponent(viewComponentTypes.button);
    v.text = t;
    v.onClick = f;
    return v;
  },
};
export class ViewComponent {
  type_ = 0;
  text_ = "";
  on_click_: FieldBase | null = null;
  text_color_ = 0;
  bg_color_ = 0;
  data: ClientData | null = null;
  constructor(
    arg: number | string | types.ViewComponent,
    data: ClientData | null = null
  ) {
    if (typeof arg === "number") {
      this.type_ = arg;
    } else if (typeof arg === "string") {
      this.type_ = viewComponentTypes.text;
      this.text_ = "";
    } else {
      this.type_ = arg.t;
      this.text_ = arg.x;
      this.on_click_ =
        arg.L != null && arg.l != null ? new FieldBase(arg.L, arg.l) : null;
      this.text_color_ = arg.c;
      this.bg_color_ = arg.b;
    }
    this.data = data;
  }
  toMessage(): types.ViewComponent {
    return {
      t: this.type,
      x: this.text,
      L: this.on_click_ == null ? null : this.on_click_.member_,
      l: this.on_click_ == null ? null : this.on_click_.field_,
      c: this.text_color_,
      b: this.bg_color_,
    };
  }
  get type() {
    return this.type_;
  }
  get text() {
    return this.text_;
  }
  set text(t: string) {
    this.text_ = t;
  }
  get onClick(): Func | null {
    if (this.on_click_ != null) {
      if (this.data != null) {
        return new Func(
          new Field(this.data, this.on_click_.member_, this.on_click_.field_)
        );
      } else {
        throw new Error("cannot get onClick: ClientData not set");
      }
    } else {
      return null;
    }
  }
  // todo: 関数を直接渡す、anonymousfunc実装
  set onClick(func: Func) {
    // if(func instanceof AnonymousFunc){
    // }else if (func instanceof Func) {
    this.on_click_ = func;
    // }else{
    //   this.onClick(new AnonymousFunc(func));
    // }
  }
  get textColor() {
    return this.text_color_;
  }
  set textColor(c: number) {
    this.text_color_ = c;
  }
  get bgColor() {
    return this.bg_color_;
  }
  set bgColor(c: number) {
    this.bg_color_ = c;
  }
}
