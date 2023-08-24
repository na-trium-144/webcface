// import * as cardKey で使う
export const value = (member: string, field: string) =>
  `${member}:value:${field}`;
export const text = (member: string) => `${member}:text`;
export const func = (member: string) => `${member}:func`;
export const log = (member: string) => `${member}:log`;