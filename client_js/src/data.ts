import { Member } from "./member.js";
import { FieldWithEvent, eventType } from "./event.js";
import { Field } from "./field.js";
import { LogLine } from "./logger.js";

export class Value extends FieldWithEvent<Value> {
  constructor(base: Field, field = "") {
    super("", base.data, base.member_, field || base.field_);
    this.eventType_ = eventType.valueChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  child(field: string): Value {
    return new Value(this, this.field_ + "." + field);
  }
  // todo
  // tryGetRecurse(){
  //   return this.data.valueStore.getRecvRecurse(this.member_, this.field_);
  // }
  tryGetVec() {
    return this.data.valueStore.getRecv(this.member_, this.field_);
  }
  tryGet() {
    const v = this.tryGetVec();
    return v != null && v.length >= 1 ? v[0] : null;
  }
  getVec() {
    const v = this.tryGetVec();
    if (v == null) {
      return [];
    } else {
      return v;
    }
  }
  get() {
    const v = this.tryGet();
    if (v == null) {
      return 0;
    } else {
      return v;
    }
  }
  set(data: number | number[] | object) {
    if (this.data.valueStore.isSelf(this.member_)) {
      if (typeof data === "number") {
        this.data.valueStore.setSend(this.field_, [data]);
        this.triggerEvent(this);
      } else if (
        Array.isArray(data) &&
        data.find((v) => typeof v !== "number") == null
      ) {
        this.data.valueStore.setSend(this.field_, data);
        this.triggerEvent(this);
      } else if (typeof data === "object" && data != null) {
        for (const [k, v] of Object.entries(data)) {
          this.child(k).set(v as number | number[] | object);
        }
      }
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  time() {
    return this.data.syncTimeStore.getRecv(this.member_) || new Date(0);
  }
}
export class Text extends FieldWithEvent<Text> {
  constructor(base: Field, field = "") {
    super("", base.data, base.member_, field || base.field_);
    this.eventType_ = eventType.textChange(this);
  }
  get member() {
    return new Member(this);
  }
  get name() {
    return this.field_;
  }
  child(field: string): Text {
    return new Text(this, this.field_ + "." + field);
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
  set(data: string | object) {
    if (this.data.textStore.isSelf(this.member_)) {
      if (typeof data === "object" && data != null) {
        for (const [k, v] of Object.entries(data)) {
          this.child(k).set(v as string | object);
        }
      } else {
        this.data.textStore.setSend(this.field_, String(data));
        this.triggerEvent(this);
      }
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
  time() {
    return this.data.syncTimeStore.getRecv(this.member_) || new Date(0);
  }
}

export class Log extends FieldWithEvent<Log> {
  constructor(base: Field) {
    super("", base.data, base.member_, "");
    this.eventType_ = eventType.logAppend(this);
  }
  get member() {
    return new Member(this);
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
