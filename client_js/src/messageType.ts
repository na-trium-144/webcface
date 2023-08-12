import {Val} from "./funcResult.js";

export const kind = {
  value: 0,
  text: 1,
  subscribe: 100,
  entry: 50,
  syncInit: 150,
  call: 151,
  callResponse: 155,
  callResult: 152,
  // entry: 153,
  funcInfo: 154,
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
  n: string;
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
  n: string;
  d: T;
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
  n: string;
  r: number;
  a: Arg[];
}
export interface Req {
  f: string;
  n: string;
}
export interface Entry {
  m: string;
  n: string;
}

export type Any =
  | SyncInit
  | Call
  | CallResponse
  | CallResult
  | Data<number>
  | Data<string>
  | FuncInfo
  | Req
  | Entry;
