import { useState, useEffect, useRef } from "react";
import { Client, Value, Text, Func } from "webcface";
import "./index.css";
import { CardItem } from "./components/card";
import { LayoutMain } from "./components/layout";

function App() {
  const cli = useRef<Client | null>(null);
  const [valueCards, setValueCards] = useState<CardItem<Value>[]>([]);
  const [textCards, setTextCards] = useState<CardItem<Text[]>[]>([]);
  const [funcCards, setFuncCards] = useState<CardItem<Func[]>[]>([]);

  useEffect(() => {
    cli.current = new Client("a");
    cli.current.membersChange.on((m) => {
      m.valuesChange.on((v) => {
        const key = `${v.member.name}:value:${v.name}`;
        setValueCards((valueCards) =>
          valueCards.concat([
            {
              key: key,
              minH: 2,
              initH: 2,
              initW: 2,
              childProps: v,
            },
          ])
        );
        v.on((v) => {
          setValueCards((valueCards) =>
            valueCards.map((c) => (c.key === key ? { ...c, childProps: v } : c))
          );
        });
      });
      m.textsChange.once((v) => {
        setTextCards((textCards) =>
          textCards.concat([
            {
              key: `${m.name}:text`,
              minH: 1,
              initH: 2,
              initW: 4,
              childProps: v.member.texts(),
            },
          ])
        );
      });
      m.textsChange.on((v) => {
        v.on(() => {
          setTextCards((textCards) => textCards.slice());
        });
      });
      m.funcsChange.once((v) => {
        setFuncCards((funcCards) =>
          funcCards.concat([
            {
              key: `${m.name}:func`,
              minH: 2,
              initH: 2,
              initW: 6,
              childProps: v.member.funcs(),
            },
          ])
        );
      });
    });
    const i = setInterval(() => {
      cli.current?.sync();
    }, 100);
    return () => clearInterval(i);
  }, []);

  return (
    <div className="p-1 h-screen">
      <LayoutMain value={valueCards} text={textCards} func={funcCards} />
    </div>
  );
}

export default App;
