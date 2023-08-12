import * as types from "./messageType.js";
import { ClientData } from "./clientData.js";
import { Member } from "./member.js";

export class FieldBase {
  data: ClientData;
  member_: string;
  field_: string;
  constructor(data: ClientData, member: string, field = "") {
    this.data = data;
    this.member_ = member;
    this.field_ = field;
  }
}


export class Value extends FieldBase {
  constructor(base: FieldBase, field = "") {
    super(base.data, base.member_, field || base.field_);
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
    if (v == null) {
      return 0;
    } else {
      return v;
    }
  }
  set(data: number) {
    if (this.data.valueStore.isSelf(this.member_)) {
      this.data.valueStore.setSend(this.member_, this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
}
export class Text extends FieldBase {
  constructor(base: FieldBase, field = "") {
    super(base.data, base.member_, field || base.field_);
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
    if (v == null) {
      return "";
    } else {
      return v;
    }
  }
  set(data: string) {
    if (this.data.textStore.isSelf(this.member_)) {
      this.data.textStore.setSend(this.member_, this.field_, data);
    } else {
      throw new Error("Cannot set data to member other than self");
    }
  }
}
