import { Value, Text, Log } from "./data.js";
import { Func } from "./func.js";
import { View } from "./view.js";
import { Field } from "./field.js";
import { FieldWithEvent, eventType } from "./event.js";

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
  log() {
    return new Log(this);
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
  get onValueEntry() {
    return new FieldWithEvent<Value>(
      eventType.valueEntry(this),
      this.data,
      this.member_
    );
  }
  get onTextEntry() {
    return new FieldWithEvent<Text>(
      eventType.textEntry(this),
      this.data,
      this.member_
    );
  }
  get onFuncEntry() {
    return new FieldWithEvent<Func>(
      eventType.funcEntry(this),
      this.data,
      this.member_
    );
  }
  get onViewEntry() {
    return new FieldWithEvent<View>(
      eventType.viewEntry(this),
      this.data,
      this.member_
    );
  }
  get onSync() {
    return new FieldWithEvent<Member>(
      eventType.sync(this),
      this.data,
      this.member_
    );
  }
}
