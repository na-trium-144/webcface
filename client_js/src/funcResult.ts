import { FieldBase } from "./data.ts";

export class FuncNotFoundError extends Error {
  constructor(base: FieldBase) {
    super(`member("${base.member_}").func("${base.field_}") is not set`);
    this.name = "FuncNotFoundError";
  }
}

export type Val = string | number | boolean | void;
export class AsyncFuncResult extends FieldBase {
  callerId: number;
  caller: string;
  // 関数が開始したらtrue, 存在しなければfalse
  // falseの場合rejectResultも自動で呼ばれる
  resolveStarted: (r: boolean) => void;
  // 結果をセットする
  resolveResult: (r: Val) => void;
  // 例外をセットする
  rejectResult: (e: any) => void;
  started: Promise<boolean>;
  result: Promise<Val>;
  constructor(callerId: number, caller: string, base: FieldBase) {
    super(base.data, base.member_, base.field_);
    this.callerId = callerId;
    this.caller = caller;
    this.started = new Promise((res) => {
      this.resolveStarted = (r: boolean) => {
        res(r);
        if (!r) {
          this.rejectResult(new FuncNotFoundError(this));
        }
      };
    });
    this.result = new Promise((res, rej) => {
      this.resolveResult = res;
      this.rejectResult = rej;
    });
  }
}
