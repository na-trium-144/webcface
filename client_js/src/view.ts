import * as types from "./message.ts";
export const viewComponentTypes = {
  text: 0,
  newLine: 1,
};
export const viewComponents = {
  newLine: () => new ViewComponent(viewComponentTypes.newLine),
};
export type ViewComponentsDiff = {[key in number]: ViewComponent};
export class ViewComponent {
  type_ = 0;
  text_ = "";
  constructor(arg: number | string | types.ViewComponent) {
    if (typeof arg === "number") {
      this.type_ = arg;
    } else if (typeof arg === "string") {
      this.type_ = viewComponentTypes.text;
      this.text_ = "";
    } else {
      this.type_ = arg.t;
      this.text_ = arg.x;
    }
  }
  equalTo(vc: ViewComponent | undefined){
    return vc != undefined && this.type_ === vc.type_ && this.text_ === vc.text_;
  }
  get type() {
    return this.type_;
  }
  get text() {
    return this.text_;
  }
}

export function getViewDiff(current: ViewComponent[], prev: ViewComponent[]){
  const diff: ViewComponentsDiff = {};
  for(let i = 0; i < current.length; i++){
    if(!current[i].equalTo(prev[i])){
      diff[i] = current[i];
    }
  }
  return diff;
}
export function mergeViewDiff(diff: ViewComponentsDiff, size: number, prev: ViewComponent[]){
  for (let i = 0; i < size; i++){
    if(diff[i] != undefined){
      if(prev.length <= i){
        prev.push(diff[i]);
      }else{
        prev[i] = diff[i];
      }
    }
  }
  while(prev.length > size){
    prev.pop();
  }
}