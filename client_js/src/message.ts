import msgpack from "@ygoe/msgpack";
import * as types from "./messageType";

export function unpack(msg: ArrayBuffer) {
  const m = msgpack.deserialize(msg) as [number, types.Any];
  // console.log("unpack:")
  // console.log(m);
  return m;
}
export function pack(kind: number, data: types.Any) {
  console.log("pack:")
  console.log([kind, data]);
  return msgpack.serialize([kind, data]);
}
