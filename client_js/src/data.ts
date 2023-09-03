import { Member } from "./member.js";
import { FieldWithEvent, eventType } from "./event.js";
import { Field } from "./field.js";

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


export class Log extends FieldWithEvent<Log> {
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
