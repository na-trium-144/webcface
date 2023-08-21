export type Val = string | number | boolean;

export interface FuncInfo {
  returnType: number;
  args: Arg[];
  funcImpl?: (...args: Val[]) => Val | Promise<Val> | void;
  // call
}

export interface Arg {
  name?: string;
  type?: number;
  init?: Val | null;
  min?: number | null;
  max?: number | null;
  option?: string[] | number[];
}
