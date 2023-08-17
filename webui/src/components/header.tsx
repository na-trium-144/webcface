import { useState, useEffect } from "react";
import version from "../libs/version";

interface Props {
  menuOpen: boolean;
  setMenuOpen: (menuOpen: boolean) => void;
}
export function Header(props: Props) {
  return (
    <nav className="bg-green-300 w-full px-2 drop-shadow-lg ">
      <div className="flex h-12 items-center space-x-4">
        <button onClick={() => props.setMenuOpen(!props.menuOpen)}>
          めにゅー
        </button>
        <div className="flex-none flex items-baseline space-x-2">
          <h1 className="text-2xl">WebCFace</h1>
          <span>ver.{version}</span>
        </div>
        <div>hoge</div>
        <div>fuga</div>
      </div>
    </nav>
  );
}
