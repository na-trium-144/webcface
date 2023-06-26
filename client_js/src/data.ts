import * as types from "./messageType";

export class Value {
  send: types.Data[];
  subsc: types.Subscribe[];
  recv: types.Recv[];
  from: string;
  name: string;
  constructor(
    send: types.Data[],
    subsc: types.Subscribe[],
    recv: types.Recv[],
    from: string,
    name: string
  ) {
    this.send = send;
    this.recv = recv;
    this.subsc = subsc;
    this.from = from;
    this.name = name;
  }
  try_get() {
    return dataGet(this) as number | null;
  }
  get() {
    const v = this.try_get();
    if (v == null) {
      return 0;
    } else {
      return v;
    }
  }
  set(data: number) {
    dataSet(this, data);
  }
}
export class Text {
  send: types.Data[];
  subsc: types.Subscribe[];
  recv: types.Recv[];
  from: string;
  name: string;
  constructor(
    send: types.Data[],
    subsc: types.Subscribe[],
    recv: types.Recv[],
    from: string,
    name: string
  ) {
    this.send = send;
    this.recv = recv;
    this.subsc = subsc;
    this.from = from;
    this.name = name;
  }
  try_get(): string | null {
    return dataGet(this) as string | null;
  }
  get() {
    const v = this.try_get();
    if (v == null) {
      return "";
    } else {
      return v;
    }
  }
  set(data: string) {
    dataSet(this, data);
  }
}
function dataGet(dataStore: Value | Text) {
  if (dataStore.from == "") {
    const s = dataStore.send.find((s) => s.n === dataStore.name);
    if (s) {
      return s.d;
    }
  } else {
    dataStore.subsc.push({ f: dataStore.from, n: dataStore.name });
  }
  const s = dataStore.recv.find(
    (s) => s.f == dataStore.from && s.n == dataStore.name
  );
  if (s) {
    return s.d;
  }
  return null;
}
function dataSet(dataStore: Value | Text, data: number | string) {
  const s = dataStore.send.find((s) => s.n === dataStore.name);
  if (s) {
    s.d = data;
  } else {
    dataStore.send.push({ n: dataStore.name, d: data });
  }
}
