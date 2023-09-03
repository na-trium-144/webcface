import { ViewComponent, ViewComponentsDiff } from "./message.js";
import isEqual from "lodash.isequal";

export const viewComponentTypes = {
  text: 0,
  newLine: 1,
  button: 2,
};
export const viewColor = {
  inherit: 0,
  black: 1,
  white: 2,
  // slate : 3,
  gray: 4,
  // zinc : 5,
  // neutral : 6,
  // stone : 7,
  red: 8,
  orange: 9,
  // amber : 10,
  yellow: 11,
  // lime : 12,
  green: 13,
  // emerald : 14,
  teal: 15,
  cyan: 16,
  // sky : 17,
  blue: 18,
  indigo: 19,
  // violet : 20,
  purple: 21,
  // fuchsia : 22,
  pink: 23,
  // rose : 24,
};

export function getViewDiff(current: ViewComponent[], prev: ViewComponent[]) {
  const diff: ViewComponentsDiff = {};
  for (let i = 0; i < current.length; i++) {
    if (!isEqual(current[i], prev[i])) {
      diff[i] = current[i];
    }
  }
  return diff;
}
export function mergeViewDiff(
  diff: ViewComponentsDiff,
  size: number,
  prev: ViewComponent[]
) {
  for (let i = 0; i < size; i++) {
    if (diff[i] != undefined) {
      if (prev.length <= i) {
        prev.push(diff[i]);
      } else {
        prev[i] = diff[i];
      }
    }
  }
  while (prev.length > size) {
    prev.pop();
  }
}
