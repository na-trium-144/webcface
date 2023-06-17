import { useState } from 'react';

const key = "RC23WebCon-fav";
const initialValue = [];

export interface FunctionFavs{
  addFav: (func_name:string)=>void;
  delFav: (func_name:string) => void;
  isFav: (func_name:string) => void;
}
export const useFunctionFavs = () => {
  const [favList, setFavList] = useState<string[]>(() => {
    if (typeof window === "undefined") return initialValue;
    const lis = JSON.parse(localStorage.getItem(key));
    if (lis == null) return [];
    else return lis
  });

  const impl_setFavs = (data:any[]) => {
    setFavList(data);
    if (typeof window !== "undefined") {
      window.localStorage.setItem(key, JSON.stringify(data));
    }
  }

  const addFav = (func_name: string) => {
    if (favList.includes(func_name)) return;
    let res = favList.slice();
    res.unshift(func_name);
    impl_setFavs(res);
  };

  const delFav = (func_name: string) => {
    if (!favList.includes(func_name)) return;
    const res = favList.filter(item => item !== func_name);
    impl_setFavs(res);
  };

  const isFav = (func_name: string) => {
    return favList.includes(func_name);
  };

  return { addFav, delFav, isFav };
}
