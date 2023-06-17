import { useContext, createContext, useEffect, useState, useRef } from "react";
interface SidebarStateI {
  sidebar: boolean;
  admin: boolean;
  cameraSelect: boolean;
  terminalLog: boolean[];
  shellFav: boolean;
  shell: boolean[];
  logSelect: boolean;
  custom: object;
  gamepad: boolean[];
}
interface SidebarStateContextI {
  sidebarState: SidebarStateI;
  setSidebarState: (callback: (state: SidebarStateI) => SidebarStateI) => void;
}
export const SidebarStateContext = createContext<SidebarStateContextI>();
export const useSidebarState = () => {
  return useContext(SidebarStateContext);
};
const initialSidebarState = {
  sidebar: true,
  admin: false,
  cameraSelect: false,
  terminalLog: [],
  shell: [],
  shellFav: false,
  logSelect: false,
  custom: {},
  gamepad: [],
};
export const SidebarStateProvider = (props) => {
  const lsKey = "RC23WebCon-Sidebar";
  const loadLsSidebarState = () => {
    try {
      return JSON.parse(window.localStorage.getItem(lsKey)) || {};
    } catch {
      return {};
    }
  };
  const saveLsSidebarState = (layouts: object) => {
    try {
      window.localStorage.setItem(lsKey, JSON.stringify(layouts));
    } catch {}
  };
  const [lsInit, setLsInit] = useState<boolean>(false);
  const [sidebarState, setSidebarState] = useState<SidebarStateContextI>({
    ...initialSidebarState,
  });
  useEffect(() => {
    setSidebarState({
      ...initialSidebarState,
      ...loadLsSidebarState(),
    });
    setLsInit(true);
  }, []);
  useEffect(() => {
    if (lsInit) {
      saveLsSidebarState(sidebarState);
    }
  }, [sidebarState, lsInit]);
  return (
    <SidebarStateContext.Provider
      value={{
        sidebarState,
        setSidebarState,
      }}
    >
      {props.children}
    </SidebarStateContext.Provider>
  );
};
