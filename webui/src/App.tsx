import { useState, useEffect, useRef } from "react";
import { Client, Member, Value, Text, Func } from "webcface";
import "./index.css";
import { CardItem } from "./components/card";
import {
  LayoutMain,
  MemberValues,
  MemberFuncs,
  MemberTexts,
} from "./components/layout";

export default function App() {
  const cli = useRef<Client | null>(null);
  const [values, setValues] = useState<MemberValues[]>([]);
  const [texts, setTexts] = useState<MemberTexts[]>([]);
  const [funcs, setFuncs] = useState<MemberFuncs[]>([]);

  useEffect(() => {
    cli.current = new Client("a");
    const onMembersChange = (m: Member) => {
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

  return (
    <div className="p-1 h-screen">
      <LayoutMain memberValues={values} memberTexts={texts} memberFuncs={funcs} />
    </div>
  );
}
