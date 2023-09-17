import * as Message from "./message.js";
import isEqual from "lodash.isequal";
import { Func, AnonymousFunc, FuncCallback } from "./func.js";
import { Member } from "./member.js";
import { ClientData } from "./clientData.js";
import { FieldWithEvent, eventType } from "./event.js";
import { Field, FieldBase } from "./field.js";

export const viewComponentTypes = {
  text: 0,
  newLine: 1,
  button: 2,
} as const;
export const viewColor = {
  inherit: 0,
  black: 1,
  white: 2,
  // slate : 3,
  gray: 4,
  // zinc : 5,
  // neutral : 6,
  // stone : 7,
  red: 8,
  orange: 9,
  // amber : 10,
  yellow: 11,
  // lime : 12,
  green: 13,
  // emerald : 14,
  teal: 15,
  cyan: 16,
  // sky : 17,
  blue: 18,
  indigo: 19,
  // violet : 20,
  purple: 21,
  // fuchsia : 22,
  pink: 23,
  // rose : 24,
} as const;

export function getViewDiff(
  current: Message.ViewComponent[],
  prev: Message.ViewComponent[]
) {
  const diff: Message.ViewComponentsDiff = {};
  for (let i = 0; i < current.length; i++) {
    if (!isEqual(current[i], prev[i])) {
      diff[i] = current[i];
    }
  }
  return diff;
}
export function mergeViewDiff(
  diff: Message.ViewComponentsDiff,
  size: number,
  prev: Message.ViewComponent[]
) {
  for (let i = 0; i < size; i++) {
    if (diff[i] != undefined) {
      if (prev.length <= i) {
        prev.push(diff[i]);
      } else {
        prev[i] = diff[i];
      }
    }
  }
  while (prev.length > size) {
    prev.pop();
  }
}

export const viewComponents = {
  newLine: (options?: ViewComponentOption) =>
    new ViewComponent(viewComponentTypes.newLine, null, options),
  text: (t: string, options?: ViewComponentOption) =>
    new ViewComponent(t, null, options),
  button: (
    t: string,
    f: Func | AnonymousFunc | FuncCallback,
    options?: ViewComponentOption
  ) => {
    const v = new ViewComponent(viewComponentTypes.button, null, options);
    v.text = t;
    v.onClick = f;
    return v;
  },
} as const;

interface ViewComponentOption {
  textColor?: number;
  bgColor?: number;
}
export class ViewComponent {
  type_ = 0;
  text_ = "";
  on_click_: FieldBase | null = null;
  on_click_tmp_: AnonymousFunc | null = null;
  text_color_ = 0;
  bg_color_ = 0;
  data: ClientData | null = null;
  constructor(
    arg: number | string | Message.ViewComponent,
    data: ClientData | null = null,
    options?: ViewComponentOption
  ) {
    if (typeof arg === "number") {
      this.type_ = arg;
    } else if (typeof arg === "string") {
      this.type_ = viewComponentTypes.text;
      this.text_ = arg;
    } else {
      this.type_ = arg.t;
      this.text_ = arg.x;
      this.on_click_ =
        arg.L != null && arg.l != null ? new FieldBase(arg.L, arg.l) : null;
      this.text_color_ = arg.c;
      this.bg_color_ = arg.b;
    }
    this.data = data;
    if (options?.textColor != undefined) {
      this.textColor = options.textColor;
    }
    if (options?.bgColor != undefined) {
      this.bgColor = options.bgColor;
    }
  }
  lockTmp(data: ClientData, field: string) {
    if (this.on_click_tmp_) {
      const f = new Func(new Field(data, data.selfMemberName, field));
      this.on_click_tmp_.lockTo(f);
      f.hidden = true;
      this.on_click_ = f;
    }
    return this;
  }
  toMessage(): Message.ViewComponent {
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
  set onClick(func: Func | AnonymousFunc | FuncCallback) {
    if (func instanceof AnonymousFunc) {
      this.on_click_tmp_ = func;
    } else if (func instanceof Func) {
      this.on_click_ = func;
    } else {
      this.onClick = new AnonymousFunc(null, func, Message.valType.none_, []);
    }
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

export class View extends FieldWithEvent<View> {
  constructor(base: Field, field = "") {
    super("", base.data, base.member_, field || base.field_);
    this.eventType_ = eventType.viewChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return (
      this.data.viewStore
        .getRecv(this.member_, this.field_)
        ?.map((v) => new ViewComponent(v, this.data)) || null
    );
  }
  get() {
    const v = this.tryGet();
    if (v === null) {
      return [];
    } else {
      return v;
    }
  }
  time() {
    return this.data.syncTimeStore.getRecv(this.member_) || new Date(0);
  }
  set(data: (ViewComponent | string | number | boolean)[]) {
    if (this.data.viewStore.isSelf(this.member_)) {
      const data2: ViewComponent[] = [];
      for (let c of data) {
        if (c instanceof ViewComponent) {
          data2.push(c);
        } else if (typeof c === "string") {
          while (c.includes("\n")) {
            const s = c.slice(0, c.indexOf("\n"));
            data2.push(viewComponents.text(s));
            data2.push(viewComponents.newLine());
            c = c.slice(c.indexOf("\n") + 1);
          }
          if (c !== "") {
            data2.push(viewComponents.text(c));
          }
        } else {
          data2.push(viewComponents.text(String(c)));
        }
      }
      this.data.viewStore.setSend(
        this.field_,
        data2.map((c, i) =>
          c.lockTmp(this.data, `${this.field_}_${i}`).toMessage()
        )
      );
      this.triggerEvent(this);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
}
