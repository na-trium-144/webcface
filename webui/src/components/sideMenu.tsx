import { useState, useEffect } from "react";
import { Client, Member, Value, Text, Func } from "webcface";
import { MemberValues, MemberTexts, MemberFuncs } from "../libs/stateTypes";
import * as cardKey from "../libs/cardKey";

interface Props {
  menuOpen: boolean;
  members: Member[];
  memberValues: MemberValues[];
  isOpened: (key: string) => boolean;
  toggleOpened: (key: string) => void;
}
export function SideMenu(props: Props) {
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
        <SideMenuMember
          key={mi}
          member={m}
          values={
            props.memberValues.find((v) => v.name === m.name)?.values || []
          }
          isOpened={props.isOpened}
          toggleOpened={props.toggleOpened}
        />
      ))}
    </nav>
  );
}

interface MemberProps {
  member: Member;
  values: Value[];
  isOpened: (key: string) => boolean;
  toggleOpened: (key: string) => void;
}
function SideMenuMember(props: MemberProps) {
  const [open, setOpen] = useState<boolean>(false);
  return (
    <>
      <div>
        <SideMenuButton
          name={props.member.name}
          onClick={() => setOpen(!open)}
        />
      </div>
      <ul className={"pl-4 " + (open ? "block " : "hidden ")}>
        {props.values.map((v, vi) => (
          <li key={vi}>
            <SideMenuButton2
              name={v.name}
              active={props.isOpened(cardKey.value(props.member.name, v.name))}
              onClick={props.toggleOpened(cardKey.value(props.member.name, v.name))}
            />
          </li>
        ))}
        <li>
          <SideMenuButton2
            name={"Text Variables"}
            active={props.isOpened(cardKey.text(props.member.name))}
            onClick={props.toggleOpened(cardKey.text(props.member.name))}
          />
        </li>
        <li>
          <SideMenuButton2
            name={"Functions"}
            active={props.isOpened(cardKey.func(props.member.name))}
            onClick={props.toggleOpened(cardKey.func(props.member.name))}
          />
        </li>
      </ul>
    </>
  );
}
interface ButtonProps {
  name: string;
  active?: boolean;
  onClick: () => void;
}
function SideMenuButton(props: ButtonProps) {
  return (
    <button className="hover:text-green-700 " onClick={props.onClick}>
      {props.name}
    </button>
  );
}
function SideMenuButton2(props: ButtonProps) {
  return (
    <button
      className={
        "w-full text-left pl-1 " +
        (props.active
          ? "bg-green-100 hover:bg-green-200 active:bg-green-300 "
          : "hover:bg-neutral-100 active:bg-neutral-200 ")
      }
      onClick={props.onClick}
    >
      {props.name}
    </button>
  );
}
