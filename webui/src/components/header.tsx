import { useState, useEffect } from "react";
import version from "../libs/version";

interface Props {
  menuOpen: boolean;
  setMenuOpen: (menuOpen: boolean) => void;
}
export function Header(props: Props) {
  return (
      <div className="flex h-full items-center space-x-4">
        <div className="flex-1 flex items-baseline space-x-2">
          <h1 className="text-2xl">WebCFace</h1>
          <span>ver.{version}</span>
        </div>
        <button onClick={() => props.setMenuOpen(!props.menuOpen)}>
          めにゅー
        </button>
      </div>
  );
}
