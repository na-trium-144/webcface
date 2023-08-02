import { useState, useEffect, useRef } from "react";
import { Client } from "webcface";
import "./index.css";

function App() {
  const cli = useRef<Client | null>(null);
  const [values, setValues] = useState<{ name: string; value: number }[]>([]);
  const [texts, setTexts] = useState<{ name: string; value: number }[]>([]);
  useEffect(() => {
    cli.current = new Client("a", "127.0.0.1", 80);
    setInterval(() => {
      setValues(
        cli.current.subjects().reduce(
          (prev, s) =>
            prev.concat(
              s.values().map((v) => ({
                name: `${s.name()}::${v.name}`,
                value: v.get(),
              }))
            ),
          []
        )
      );
      setTexts(
        cli.current.subjects().reduce(
          (prev, s) =>
            prev.concat(
              s.texts().map((v) => ({
                name: `${s.name()}::${v.name}`,
                value: v.get(),
              }))
            ),
          []
        )
      );
      cli.current.send();
    }, 100);
  }, []);

  return (
    <div className="p-1">
      <h3 className="text-lg">values</h3>
      {values.map((v) => (
        <div
          key={v.name}
          className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 w-60 h-60"
        >
          <h6 className="flex-none mx-auto font-semibold">{v.name}</h6>
          <div className="flex-auto">{v.value}</div>
        </div>
      ))}
      <h3 className="text-lg">texts</h3>
      {texts.map((v) => (
        <div
          key={v.name}
          className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 w-60 h-24"
        >
          <h6 className="flex-none mx-auto font-semibold">{v.name}</h6>
          <div className="flex-auto">{v.value}</div>
        </div>
      ))}
    </div>
  );
}

export default App;
