import { useState, useEffect, useRef } from "react";
import { Client, Member } from "webcface";
import "./index.css";
import { LayoutMain } from "./components/layout";
import { Header } from "./components/header";
import { SideMenu } from "./components/sideMenu";
import {
  MemberValues,
  MemberTexts,
  MemberFuncs,
  MemberLogs,
} from "./libs/stateTypes";

export default function App() {
  const [client, setClient] = useState<Client>(new Client("a"));
  const [members, setMembers] = useState<Member[]>([]);
  const [values, setValues] = useState<MemberValues[]>([]);
  const [texts, setTexts] = useState<MemberTexts[]>([]);
  const [funcs, setFuncs] = useState<MemberFuncs[]>([]);
  const [logs, setLogs] = useState<MemberLogs[]>([]);
  useEffect(() => {
    const i = setInterval(() => {
      client.sync();
    }, 100);
    return () => clearInterval(i);
  }, [client]);
  useEffect(() => {
    const onMembersChange = (m: Member) => {
      if (members.find((m2) => m2.name === m.name)) {
        // 本来はこの条件要らないようにするはず
        return;
      }
      setMembers((members) => members.concat([m]));
      // 本来は初期値入れなくてもイベントが初期値に対して反応するようにしたい
      setValues((values) =>
        values.concat([{ name: m.name, values: m.values() }])
      );
      setTexts((values) => values.concat([{ name: m.name, texts: m.texts() }]));
      setFuncs((values) => values.concat([{ name: m.name, funcs: m.funcs() }]));
      setLogs((logs) => logs.concat([{ name: m.name, logs: m.logs().get() }]));
      m.valuesChange.on(() =>
        setValues((values) => {
          values.find((e) => e.name === m.name).values = m.values();
          return values.slice();
        })
      );
      m.textsChange.on(() =>
        setTexts((texts) => {
          texts.find((e) => e.name === m.name).texts = m.texts();
          return texts.slice();
        })
      );
      m.funcsChange.on(() =>
        setFuncs((funcs) => {
          funcs.find((e) => e.name === m.name).funcs = m.funcs();
          return funcs.slice();
        })
      );
      m.logs().on(() =>
        setLogs((logs) => {
          logs.find((e) => e.name === m.name).logs = m.logs().get();
          return logs.slice();
        })
      );
    };
    client.membersChange.on(onMembersChange);
    return () => {
      client.membersChange.off(onMembersChange);
    };
  }, [client, members]);

  const [menuOpen, setMenuOpen] = useState<boolean>(false);
  const [openedCards, setOpenedCards] = useState<string[]>([]);
  const isOpened = (key: string) => openedCards.includes(key);
  const openedOrder = (key: string) => openedCards.indexOf(key) || 0;
  const toggleOpened = (key: string) => {
    if (openedCards.includes(key)) {
      setOpenedCards(openedCards.filter((n) => n !== key));
    } else {
      setOpenedCards(openedCards.concat([key]));
    }
  };
  const moveOrder = (key: string) => {
    setOpenedCards(openedCards.filter((n) => n !== key).concat([key]));
  };

  return (
    <div className="min-h-screen h-max bg-neutral-100">
      <nav className="bg-green-300 w-full h-12 px-2 drop-shadow-lg">
        <Header menuOpen={menuOpen} setMenuOpen={setMenuOpen} />
      </nav>
      <nav
        className={
          "absolute top-10 right-2 w-72 h-max max-h-[75%] p-2 " +
          "rounded-lg shadow-lg overflow-x-hidden overflow-y-auto bg-white " +
          "transition duration-100 origin-top-right " +
          (menuOpen
            ? "ease-out opacity-100 scale-100 z-[1000] "
            : "ease-in opacity-0 scale-90 -z-10 ")
        }
      >
        <SideMenu
          members={members}
          memberValues={values}
          isOpened={isOpened}
          toggleOpened={toggleOpened}
        />
      </nav>
      <main className="p-2">
        <LayoutMain
          isOpened={isOpened}
          openedOrder={openedOrder}
          moveOrder={moveOrder}
          memberLogs={logs}
          memberValues={values}
          memberTexts={texts}
          memberFuncs={funcs}
        />
      </main>
    </div>
  );
}
