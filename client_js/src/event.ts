import { ClientData } from "./clientData.js";
import { FieldBase, Field } from "./field.js";

export const eventType = {
  memberEntry: () => "memberEntry",
  sync: (b: FieldBase) => JSON.stringify(["sync", b.member_]),
  valueEntry: (b: FieldBase) => JSON.stringify(["valueEntry", b.member_]),
  textEntry: (b: FieldBase) => JSON.stringify(["textEntry", b.member_]),
  funcEntry: (b: FieldBase) => JSON.stringify(["funcEntry", b.member_]),
  viewEntry: (b: FieldBase) => JSON.stringify(["viewEntry", b.member_]),
  valueChange: (b: FieldBase) =>
    JSON.stringify(["valueChange", b.member_, b.field_]),
  textChange: (b: FieldBase) =>
    JSON.stringify(["textChange", b.member_, b.field_]),
  viewChange: (b: FieldBase) =>
    JSON.stringify(["viewChange", b.member_, b.field_]),
  logAppend: (b: FieldBase) => JSON.stringify(["logAppend", b.member_]),
};
type EventListener<TargetType> = (target: TargetType) => void;
export class FieldWithEvent<TargetType> extends Field {
  eventType_: string;
  // onAppend: () => void;
  constructor(
    eventType: string,
    data: ClientData,
    member: string,
    field = ""
    // onAppend: () => void = () => undefined
  ) {
    super(data, member, field);
    this.eventType_ = eventType;
    // this.onAppend = onAppend;
  }
  triggerEvent(arg: TargetType){
    this.data.eventEmitter.emit(this.eventType_, arg);
  }
  addListener(listener: EventListener<TargetType>) {
    this.data.eventEmitter.addListener(this.eventType_, listener);
    // this.onAppend();
  }
  on(listener: EventListener<TargetType>) {
    this.addListener(listener);
  }
  once(listener: EventListener<TargetType>) {
    this.data.eventEmitter.once(this.eventType_, listener);
    // this.onAppend();
  }
  removeListener(listener: EventListener<TargetType>) {
    this.data.eventEmitter.removeListener(this.eventType_, listener);
  }
  off(listener: EventListener<TargetType>) {
    this.removeListener(listener);
  }
  removeAllListeners() {
    this.data.eventEmitter.removeAllListeners(this.eventType_);
  }
}
