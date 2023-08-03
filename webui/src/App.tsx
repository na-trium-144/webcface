import { useState, useEffect, useRef } from "react";
import { Client, Value, Text, Func } from "webcface";
import "./index.css";
import { Card, CardItem } from "./components/card";
import { LayoutMain } from "./components/layout";

function App() {
  const cli = useRef<Client | null>(null);
  const [valueCards, setValueCards] = useState<CardItem<Value>[]>([]);
  const [textCards, setTextCards] = useState<CardItem<Text[]>[]>([]);
  const [funcCards, setFuncCards] = useState<CardItem<Func[]>[]>([]);

  useEffect(() => {
    cli.current = new Client("a", "127.0.0.1", 80);
    const i = setInterval(() => {
      const values: CardItem<Value>[] = [];
      const texts: CardItem<Text[]>[] = [];
      const funcs: CardItem<Func[]>[] = [];
      for (const s of cli.current.subjects()) {
        for (const v of s.values()) {
          values.push({
            key: `${s.name()}:value:${v.name}`,
            minH: 2,
            initH: 2,
            initW: 2,
            childProps: v,
          });
        }
        if (s.texts().length > 0) {
          texts.push({
            key: `${s.name()}:text`,
            minH: 1,
            initH: 2,
            initW: 4,
            childProps: s.texts(),
          });
        }
        if (s.funcs().length > 0) {
          funcs.push({
            key: `${s.name()}:func`,
            minH: 2,
            initH: 2,
            initW: 6,
            childProps: s.funcs(),
          });
        }
      }
      setValueCards(values);
      setTextCards(texts);
      setFuncCards(funcs);
      cli.current.send();
    }, 100);
    return () => clearInterval(i);
  }, [setFuncCards, setTextCards, setValueCards]);

  return (
    <div className="p-1 h-screen">
      <LayoutMain value={valueCards} text={textCards} func={funcCards} />
    </div>
  );
}

export default App;
