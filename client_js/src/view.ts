import * as Message from "./message.js";
import isEqual from "lodash.isequal";
import { Func } from "./func.js";
import { Member } from "./member.js";
import { ClientData } from "./clientData.js";
import { FieldWithEvent, eventType } from "./event.js";
import { Field, FieldBase } from "./field.js";

export const viewComponentTypes = {
  text: 0,
  newLine: 1,
  button: 2,
};
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
};

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
    arg: number | string | Message.ViewComponent,
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
