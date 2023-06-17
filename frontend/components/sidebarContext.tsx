import { useContext, createContext, useEffect, useState, useRef } from "react";
interface SidebarStateI {
  admin: boolean;
  cameraSelect: boolean;
  terminalLog: boolean[];
  shellFav: boolean;
  shell: boolean[];
  logSelect: boolean;
  custom: object;
}
interface SidebarStateContextI {
  sidebarState: SidebarStateI;
  setSidebarState: (callback: (state: SidebarStateI) => SidebarStateI) => void;
}
const SidebarStateContext = createContext<SidebarStateContextI>();
export const useSidebarState = () => {
  return useContext(SidebarStateContext);
};
export const SidebarStateProvider = (props) => {
  const [sidebarState, setSidebarState] = useState<SidebarStateContextI>({
    admin: false,
    cameraSelect: false,
    terminalLog: [],
    shell: [],
    shellFav: false,
    logSelect: false,
    custom: {},
  });
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
