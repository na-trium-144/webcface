import { ClientData } from "./clientData.js";

export class FieldBase {
  member_: string;
  field_: string;
  constructor(member: string, field = "") {
    this.member_ = member;
    this.field_ = field;
  }
}
export class Field extends FieldBase {
  data: ClientData;
  constructor(data: ClientData, member: string, field = "") {
    super(member, field);
    this.data = data;
  }
}
