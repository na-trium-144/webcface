import { useState, useEffect, useRef } from "react";
import { Client } from "webcface";
import "./index.css";
import "react-grid-layout/css/styles.css";
import "react-resizable/css/styles.css";
import { Responsive, WidthProvider } from "react-grid-layout";
const ResponsiveGridLayout = WidthProvider(Responsive);
import { getFromLS, saveToLS } from "./libs/ls";

function App() {
  const cli = useRef<Client | null>(null);
  const [values, setValues] = useState<{ name: string; value: number }[]>([]);
  const [texts, setTexts] = useState<{ name: string; value: string }[]>([]);
  const [funcs, setFuncs] = useState<
    { from: string; name: string; returnType: number; argsType: number[] }[]
  >([]);
  const [layouts, setLayouts] = useState<object>({});
  const [layoutsLoadDone, setLayoutsLoadDone] = useState<boolean>(false);
  useEffect(() => {
    setLayouts(getFromLS("layouts") || {});
    setLayoutsLoadDone(true);
  }, []);

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

  const onLayoutChange = (layout, layouts: object) => {
    if (layoutsLoadDone) {
      saveToLS("layouts", layouts);
      setLayouts(layouts);
    }
  };

  return (
    <div className="p-1 h-screen">
      <ResponsiveGridLayout
        className="layout h-full overflow-hidden"
        layouts={layouts}
        breakpoints={{ xxl: 1536, xl: 1280, lg: 1024, md: 768, sm: 640, xs: 0 }}
        cols={{ xx: 15, xl: 13, lg: 10, md: 7, sm: 6, xs: 2 }}
        rowHeight={100}
        onLayoutChange={onLayoutChange}
        allowOverlap
        compactType={null}
        autoSize={false}
      >
        {values.map((v) => (
          <div
            key={v.name}
            data-grid={{ w: 2, h: 2, x: 0, y: 0, minW: 2, minH: 2 }}
            className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 bg-white"
          >
            <h6 className="flex-none mx-auto font-semibold">{v.name}</h6>
            <div className="flex-auto">{v.value}</div>
          </div>
        ))}

        <div
          key={"texts"}
          data-grid={{ w: 4, h: 2, x: 0, y: 0, minW: 2, minH: 1 }}
          className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 bg-white"
        >
          <h6 className="flex-none mx-auto font-semibold">Text Status</h6>
          <div className="flex-auto">
            <ul className="list-none">
              {texts.map((v) => (
                <li key={v.name}>
                  {v.name} = {v.value}
                </li>
              ))}
            </ul>
          </div>
        </div>
        <div
          key="funcs"
          data-grid={{ w: 6, h: 2, x: 0, y: 0, minW: 2, minH: 1 }}
          className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 bg-white"
        >
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
      </ResponsiveGridLayout>
    </div>
  );
}

export default App;
