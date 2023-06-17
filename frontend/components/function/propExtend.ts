
// TとPを合成した型を返す。TとPに同名のキーが存在した場合はPのものが優先される。
export type Extend<T, P> = Pick<T, Exclude<keyof T, keyof P>> & P
