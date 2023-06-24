export const kind = {
  name: 150,
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

export type Any = Name | Data | Recv | Subscribe;
