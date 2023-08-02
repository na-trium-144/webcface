export const kind = {
  name: 150,
  call: 151,
  callResponse: 152,
  entry: 153,
  funcInfo: 154,
  value: 0,
  text: 1,
  recv: 50,
  subscribe: 100,
};
export const argType = {
  none_: 0,
  string_: 1,
  boolean_: 2,
  int_: 3,
  float_: 4,
};
export interface Name {
  n: string;
}
export interface Data {
  n: string;
  d: string | number;
}
export interface Recv {
  f: string;
  n: string;
  d: string | number;
}
export interface Subscribe {
  f: string;
  n: string;
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
  f: boolean;
  e: boolean;
  r: string | number | boolean | void;
}
export interface Entry {
  f: string;
  v: { n: string }[];
  t: { n: string }[];
  u: FuncInfo[];
}
export interface FuncInfo {
  n: string;
  r: number;
  a: number[];
}

export type Any =
  | Name
  | Data
  | Recv
  | Subscribe
  | Call
  | CallResponse
  | Entry
  | FuncInfo;
