import msgpack from "@ygoe/msgpack";
import { Val } from "./func.js";

export function unpack(msg: ArrayBuffer) {
  if (msg.byteLength === 0) {
    return [];
  }
  const m = msgpack.deserialize(msg) as any[];
  const ret: AnyMessage[] = [];
  for (let i = 0; i < m.length; i += 2) {
    ret.push({ ...(m[i + 1] as AnyMessage), kind: m[i] as number });
  }
  return ret;
}
export function pack(data: AnyMessage[]) {
  const sendData: any[] = [];
  for (let i = 0; i < data.length; i++) {
    sendData.push(data[i].kind);
    const e: { kind?: number } = { ...data[i] };
    delete e.kind;
    sendData.push(e);
  }
  return msgpack.serialize(sendData);
}

export const kind = {
  value: 0,
  text: 1,
  view: 3,
  valueEntry: 25,
  textEntry: 26,
  viewEntry: 28,
  valueReq: 50,
  textReq: 51,
  viewReq: 53,
  valueRes: 75,
  textRes: 76,
  viewRes: 78,
  syncInit: 100,
  call: 101,
  callResponse: 102,
  callResult: 103,
  funcInfo: 104,
  log: 105,
  logReq: 106,
  sync: 107,
} as const;

export const valType = {
  none_: 0,
  string_: 1,
  boolean_: 2,
  bool_: 2,
  int_: 3,
  float_: 4,
  number_: 4,
} as const;

export interface Value {
  kind: 0;
  f: string;
  d: number[];
}
export interface Text {
  kind: 1;
  f: string;
  d: string;
}
export type ViewComponentsDiff = {
  [key in string]: ViewComponent;
};
export interface ViewComponent {
  t: number;
  x: string;
  L: string | null;
  l: string | null;
  c: number;
  b: number;
}
export interface View {
  kind: 3;
  f: string;
  d: ViewComponentsDiff;
  l: number;
}
export interface Entry {
  kind: 25 | 26 | 28;
  m: number;
  f: string;
}
export interface Req {
  kind: 50 | 51 | 53;
  M: string;
  f: string;
  i: number;
}
export interface ValueRes {
  kind: 75;
  i: number;
  f: string;
  d: number[];
}
export interface TextRes {
  kind: 76;
  i: number;
  f: string;
  d: string;
}
export interface ViewRes {
  kind: 78;
  i: number;
  f: string;
  d: ViewComponentsDiff;
  l: number;
}

export interface SyncInit {
  kind: 100;
  M: string;
  m: number;
}
export interface Sync {
  kind: 107;
  m: number;
  t: number;
}
export interface Call {
  kind: 101;
  i: number;
  c: number;
  r: number;
  f: string;
  a: Val[];
}
export interface CallResponse {
  kind: 102;
  i: number;
  c: number;
  s: boolean;
}
export interface CallResult {
  kind: 103;
  i: number;
  c: number;
  e: boolean;
  r: Val;
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
  kind: 104;
  m: number;
  f: string;
  r: number;
  a: Arg[];
}

export interface LogLine {
  v: number;
  t: number;
  m: string;
}
export interface Log {
  kind: 105;
  m: number;
  l: LogLine[];
}
export interface LogReq {
  kind: 106;
  M: string;
}
export interface Unknown {
  kind: number;
}
export type AnyMessage =
  | Value
  | Text
  | View
  | Req
  | Entry
  | ValueRes
  | TextRes
  | ViewRes
  | SyncInit
  | Sync
  | Call
  | CallResponse
  | CallResult
  | FuncInfo
  | Log
  | LogReq
  | Unknown;
