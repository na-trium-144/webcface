import { Client, Value, Text, Func } from "webcface";

export interface MemberValues {
  name: string;
  values: Value[];
}
export interface MemberTexts {
  name: string;
  texts: Text[];
}
export interface MemberFuncs {
  name: string;
  funcs: Func[];
}