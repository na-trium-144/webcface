import msgpack from "@ygoe/msgpack";
import * as types from "./messageType.js";

export function unpack(msg: ArrayBuffer) {
  const m = msgpack.deserialize(msg) as [number, types.Any];
  return m;
}
export function pack(kind: number, data: types.Any) {
  return msgpack.serialize([kind, data]);
}
