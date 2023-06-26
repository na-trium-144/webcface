export const kind = {
  name: 150,
  call: 151,
  call_response: 152,
  value: 0,
  text: 1,
  recv: 50,
  subscribe: 100,
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
  a: string[];
}
export interface CallResponse {
  i: number;
  c: string;
  f: bool;
  e: bool;
  r: string;
}

export type Any = Name | Data | Recv | Subscribe | Call | CallResponse;
