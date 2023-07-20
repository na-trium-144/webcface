import { useState, useEffect, useRef } from "react";

import { Client } from "webcface";

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
    <>
      <h3>values</h3>
      <ul>
        {values.map((v) => (
          <li key={v.name}>
            {v.name} = {v.value}
          </li>
        ))}
      </ul>
      <h3>texts</h3>
      <ul>
        {texts.map((v) => (
          <li key={v.name}>
            {v.name} = {v.value}
          </li>
        ))}
      </ul>
    </>
  );
}

export default App;
