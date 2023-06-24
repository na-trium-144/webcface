import msgpack from "@ygoe/msgpack";
import * as types from "./messageType";

export function unpack(msg: ArrayBuffer) {
  return msgpack.deserialize(msg) as [number, types.Any];
}
export function pack(kind: number, data: types.Any) {
  return msgpack.serialize([kind, data]);
}
