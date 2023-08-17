import { useState, useEffect } from "react";
import { Client, Member, Value, Text, Func } from "webcface";
import { MemberValues, MemberTexts, MemberFuncs } from "../libs/stateTypes";

interface Props {
  menuOpen: boolean;
  members: Member[];
  memberValues: MemberValues[];
}
export function SideMenu(props: Props) {
  console.log(props.menuOpen);
  return (
    <nav
      className={
        "fixed top-12 left-0 w-48 h-full p-2 z-10 " +
        "overflow-y-auto overflow-x-hidden bg-white drop-shadow-lg " +
        "transition ease-in-out duration-300 " +
        (props.menuOpen ? "translate-x-0 " : "-translate-x-full ")
      }
    >
      {props.members.map((m, mi) => (
        <h4 className="" key={mi}>
          {m}
        </h4>
      ))}
    </nav>
  );
}
