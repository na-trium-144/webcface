import { useState, useEffect, useRef } from "react";
import { Client, Member, Value, Text, Func } from "webcface";
import "./index.css";
import { LayoutMain } from "./components/layout";
import { Header } from "./components/header";
import { SideMenu } from "./components/sideMenu";
import { MemberValues, MemberTexts, MemberFuncs } from "./libs/stateTypes";

export default function App() {
  const cli = useRef<Client | null>(null);
  const [members, setMembers] = useState<Member[]>([]);
  const [values, setValues] = useState<MemberValues[]>([]);
  const [texts, setTexts] = useState<MemberTexts[]>([]);
  const [funcs, setFuncs] = useState<MemberFuncs[]>([]);

  useEffect(() => {
    cli.current = new Client("a");
    const onMembersChange = (m: Member) => {
      setMembers((members) => members.concat([m]));
      setValues((values) => values.concat([{ name: m.name, values: [] }]));
      setTexts((values) => values.concat([{ name: m.name, texts: [] }]));
      setFuncs((values) => values.concat([{ name: m.name, funcs: [] }]));
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
    };
    cli.current.membersChange.on(onMembersChange);
    const i = setInterval(() => {
      cli.current?.sync();
    }, 100);
    return () => {
      clearInterval(i);
      cli.current.membersChange.off(onMembersChange);
    };
  }, []);

  const [menuOpen, setMenuOpen] = useState<boolean>(false);
  const [openedCards, setOpenedCards] = useState<string[]>([]);
  const isOpened = (key: string) => openedCards.includes(key);
  const openedOrder = (key: string) => openedCards.indexOf(key) || 0;
  const toggleOpened = (key: string) => () => {
    if (openedCards.includes(key)) {
      setOpenedCards(openedCards.filter((n) => n !== key));
    } else {
      setOpenedCards(openedCards.concat([key]));
    }
  };

  return (
    <div className="h-screen bg-neutral-100">
      <Header menuOpen={menuOpen} setMenuOpen={setMenuOpen} />
      <SideMenu
        menuOpen={menuOpen}
        members={members}
        memberValues={values}
        isOpened={isOpened}
        toggleOpened={toggleOpened}
      />
      <LayoutMain
        isOpened={isOpened}
        openedOrder={openedOrder}
        memberValues={values}
        memberTexts={texts}
        memberFuncs={funcs}
      />
    </div>
  );
}
