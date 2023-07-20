import * as types from "./messageType.js";

export interface DataStore<T> {
  dataSend: Map<string, T>;
  dataRecv: Map<string, Map<string, T>>;
  entry: Map<string, string[]>;
  req: Map<string, Map<string, boolean>>;
  reqSend: Map<string, Map<string, boolean>>;
}
export const emptyStore = () => ({
  dataSend: new Map(),
  dataRecv: new Map(),
  entry: new Map(),
  req: new Map(),
  reqSend: new Map(),
});

export class Value {
  store: DataStore<number | string>;
  from: string;
  name: string;
  constructor(store: DataStore<number | string>, from: string, name: string) {
    this.store = store;
    this.from = from;
    this.name = name;
  }
  tryGet() {
    return dataGet(this.store, this.from, this.name) as number | null;
  }
  get() {
    const v = this.tryGet();
    if (v == null) {
      return 0;
    } else {
      return v;
    }
  }
  set(data: number) {
    dataSet(this.store, this.from, this.name, data);
  }
}
export class Text {
  store: DataStore<number | string>;
  from: string;
  name: string;
  constructor(store: DataStore<number | string>, from: string, name: string) {
    this.store = store;
    this.from = from;
    this.name = name;
  }
  tryGet() {
    return dataGet(this.store, this.from, this.name) as string | null;
  }
  get() {
    const v = this.tryGet();
    if (v == null) {
      return "";
    } else {
      return v;
    }
  }
  set(data: string) {
    dataSet(this.store, this.from, this.name, data);
  }
}
function dataGet(
  store: DataStore<number | string>,
  from: string,
  name: string
) {
  if (from === "") {
    const s = store.dataSend.get(name);
    if (s) {
      return s;
    }
  }

  let hasValue = false;
  let value: number | string | null = null;
  const s = store.dataRecv.get(from);
  if (s) {
    const m = s.get(name);
    if (m) {
      value = m;
      hasValue = true;
    }
  }
  if (from !== "" && !hasValue) {
    const m = store.req.get(from);
    if (m) {
      m.set(name, true);
    } else {
      store.req.set(from, new Map([[name, true]]));
    }
    const ms = store.reqSend.get(from);
    if (ms) {
      ms.set(name, true);
    } else {
      store.reqSend.set(from, new Map([[name, true]]));
    }
  }
  return value;
}
function dataSet(
  store: DataStore<number | string>,
  from: string,
  name: string,
  data: number | string
) {
  store.dataSend.set(name, data);
  const m = store.dataRecv.get("");
  if (m) {
    m.set(name, data);
  } else {
    store.dataRecv.set("", new Map([[name, data]]));
  }
}
