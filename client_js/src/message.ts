import msgpack from "@ygoe/msgpack";
import { Val } from "./funcInfo.js";

export function unpack(msg: ArrayBuffer) {
  const m = msgpack.deserialize(msg) as [number, AnyMessage];
  return m;
}
export function pack(kind: number, data: AnyMessage) {
  return msgpack.serialize([kind, data]);
}

export const kind = {
  value: 0,
  text: 1,
  view: 3,
  entry: 50,
  req: 100,
  syncInit: 150,
  call: 151,
  callResponse: 155,
  callResult: 152,
  funcInfo: 154,
  log: 156,
  logReq: 157,
};

export const argType = {
  none_: 0,
  string_: 1,
  boolean_: 2,
  bool_: 2,
  int_: 3,
  float_: 4,
  number_: 4,
};

export interface SyncInit {
  m: string;
}
export interface Call {
  i: number;
  c: string;
  r: string;
  f: string;
  a: Val[];
}
export interface CallResponse {
  i: number;
  c: string;
  s: boolean;
}
export interface CallResult {
  i: number;
  c: string;
  e: boolean;
  r: Val;
}
export interface Data<T> {
  m: string;
  f: string;
  d: T;
}
export interface ViewComponent {
  t: number;
  x: string;
}
export interface View {
  m: string;
  f: string;
  d: { [key in number]: ViewComponent };
  l: number;
}
export interface Arg {
  n: string;
  t: number;
  i: Val | null;
  m: number | null;
  x: number | null;
  o: number[] | string[];
}
export interface FuncInfo {
  m: string;
  f: string;
  r: number;
  a: Arg[];
}
export interface Req {
  m: string;
  f: string;
}
export interface Entry {
  m: string;
  f: string;
}
export interface LogLine {
  v: number;
  t: number;
  m: string;
}
export interface Log {
  m: string;
  l: LogLine[];
}
export interface LogReq {
  m: string;
}

export type AnyMessage =
  | SyncInit
  | Call
  | CallResponse
  | CallResult
  | Data<number>
  | Data<string>
  | View
  | FuncInfo
  | Req
  | Entry
  | Log
  | LogReq;
