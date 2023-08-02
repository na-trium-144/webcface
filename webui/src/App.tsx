import { useState, useEffect, useRef } from "react";
import { Client } from "webcface";
import "./index.css";

function App() {
  const cli = useRef<Client | null>(null);
  const [values, setValues] = useState<{ name: string; value: number }[]>([]);
  const [texts, setTexts] = useState<{ name: string; value: string }[]>([]);
  const [funcs, setFuncs] = useState<
    { from: string; name: string; returnType: number; argsType: number[] }[]
  >([]);
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
      setFuncs(
        cli.current.subjects().reduce(
          (prev, s) =>
            prev.concat(
              s.funcs().map((v) => ({
                from: s.name(),
                name: v.name,
                returnType: v.returnType(),
                argsType: v.argsType(),
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
      <h3 className="text-lg">funcs</h3>
      <div className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 w-auto h-40">
        <h6 className="flex-none mx-auto font-semibold">Funcs</h6>
        <div className="flex-auto overflow-y-auto">
          <ul className="list-none">
            {funcs.map((v) => (
              <li key={`${v.from}::${v.name}`} className="">
                <span className="">
                  {v.from}::{v.name}
                </span>
                <span className="pl-1">(</span>
                {v.argsType.map((a, i) => (
                  <>
                    {i > 0 && <span className="pl-1 pr-1">,</span>}
                    <input
                      className="border-0 border-b outline-0 focus:border-b-2 border-neutral-200 hover:border-neutral-500 focus:border-black px-1"
                      key={i}
                      size="10"
                    />
                  </>
                ))}
                <span className="pr-2">)</span>
                <button
                  className="rounded-full border border-green-300 shadow-md bg-green-100 px-2 my-1"
                  onClick={() =>
                    // todo: 引数
                    void cli.current.subject(v.from).func(v.name).run()
                  }
                >
                  Run
                </button>
              </li>
            ))}
          </ul>
        </div>
      </div>
    </div>
  );
}

export default App;
