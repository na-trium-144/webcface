const lsKey = "webcface-webui";

function getLS() {
  let ls = {};
  if (global != undefined && global.localStorage) {
    try {
      ls = JSON.parse(global.localStorage.getItem(lsKey) || "{}") as object;
    } catch (e) {
      /*Ignore*/
    }
  }
  return ls;
}
export function getFromLS(key: string) {
  const ls = getLS();
  // console.log("load", ls[key]);
  return (ls[key] as object) || null;
}

export function saveToLS(key: string, value: object) {
  if (global != undefined && global.localStorage) {
    const ls = getLS();
    ls[key] = value;
    global.localStorage.setItem(lsKey, JSON.stringify(ls));
    // console.log("save", ls)
  }
}
