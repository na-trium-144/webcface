import { Member } from "./member.js";
import { FieldWithEvent, eventType } from "./event.js";
import { Field } from "./field.js";
import { LogLine } from "./logger.js";

export class Value extends FieldWithEvent<Value> {
  value_: number | null;
  time_: Date | null;
  constructor(base: Field, field = "") {
    super("", base.data, base.member_, field || base.field_);
    this.eventType_ = eventType.valueChange(this);
    this.value_ = this.data.valueStore.getRecv(this.member_, this.field_);
    this.time_ = this.data.syncTimeStore.getRecv(this.member_);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.value_;
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
  time() {
    return this.time_ || new Date(0);
  }
}
export class Text extends FieldWithEvent<Text> {
  value_: string | null;
  time_: Date | null;
  constructor(base: Field, field = "") {
    super("", base.data, base.member_, field || base.field_);
    this.eventType_ = eventType.textChange(this);
    this.value_ = this.data.textStore.getRecv(this.member_, this.field_);
    this.time_ = this.data.syncTimeStore.getRecv(this.member_);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.value_;
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
  time() {
    return this.time_ || new Date(0);
  }
}

export class Log extends FieldWithEvent<LogLine> {
  value_: LogLine[] | null;
  constructor(base: Field) {
    super("", base.data, base.member_, "");
    this.eventType_ = eventType.logAppend(this);
    this.value_ = this.data.logStore.getRecv(this.member_);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  tryGet() {
    return this.value_;
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
