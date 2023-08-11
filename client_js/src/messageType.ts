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
  int_: 3,
  float_: 4,
};
export interface SyncInit {
  m: string;
}
export interface Call {
  i: number;
  c: string;
  r: string;
  n: string;
  a: (string | number | boolean)[];
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
  r: string | number | boolean;
}
export interface Data {
  m: string;
  n: string;
  d: string | number;
}
export interface Arg {
  n: string;
  t: number;
  i: number | string | null;
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
  | Data
  | FuncInfo
  | Req
  | Entry;
