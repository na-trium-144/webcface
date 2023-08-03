import { useState, useEffect, useRef } from "react";
import { Client, Value, Text, Func } from "webcface";
import "../index.css";
import "react-grid-layout/css/styles.css";
import "react-resizable/css/styles.css";
import { Responsive, WidthProvider, Layout, Layouts} from "react-grid-layout";
const ResponsiveGridLayout = WidthProvider(Responsive);
import { getFromLS, saveToLS } from "../libs/ls";
import { Card, CardItem } from "./card";

interface Props {
  value: CardItem<Value>[];
  text: CardItem<Text[]>[];
  func: CardItem<Func[]>[];
}

export function LayoutMain(props: Props) {
  const [layouts, setLayouts] = useState<Layouts>({});
  const [lsLayout, setLsLayout] = useState<Layout[]>({});
  const [layoutsLoadDone, setLayoutsLoadDone] = useState<boolean>(false);
  const prevLayout = useRef<Layout[]>([]);
  useEffect(() => {
    const ls = getFromLS("layout") || [];
    setLsLayout(ls);
    setLayouts({xs: ls,sm: ls, md: ls, lg: ls, xl: ls, xxl: ls});
    setLayoutsLoadDone(true);
  }, []);
  const onLayoutChange = (layout: Layout[], layouts: Layouts) => {
    console.log("layoutchange", layout)
    for(let nli = 0; nli < layout.length; ++nli){
      const nl = layout[nli];
      const pli = prevLayout.current.findIndex((pl) => pl.i === nl.i);
      const lli = lsLayout.findIndex((ll) => ll.i === nl.i);
      if(pli >= 0){
        prevLayout.current[pli] = {...nl};
      }else{
        if(lli >= 0){
          layout[nli] = lsLayout[lli];
        }
        prevLayout.current.push(lsLayout[lli]);
      }
    }
    if (layoutsLoadDone) {
      saveToLS("layout", layout);
      setLsLayout(layout);
      setLayouts({xs: layout,sm: layout, md: layout, lg: layout, xl: layout, xxl: layout});
    }
  };

  const dataGrid = (c: CardItem<any>) => {
    return {
      w: c.initW,
      h: c.initH,
      x: 0,
      y: 0,
      minW: 2,
      minH: c.minH,
    };
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
        {props.value.map((c) => (
          <div key={c.key} data-grid={dataGrid(c)}>
            <Card title={c.childProps.name}>{c.childProps.get()}</Card>
          </div>
        ))}
        {props.text.map((c) => (
          <div
            key={c.key}
            data-grid={dataGrid(c)}
          >
            <Card title="Text Status">
              <ul className="list-none">
                {c.childProps.map((v) => (
                  <li key={v.name}>
                    {v.from}::{v.name} = {v.get()}
                  </li>
                ))}
              </ul>
            </Card>
          </div>
        ))}
        {props.func.map((c) => (
          <div
            key={c.key}
            data-grid={dataGrid(c)}
          >
            <Card title="Functions">
              <ul className="list-none">
                {c.childProps.map((v) => (
                  <li key={`${v.from}::${v.name}`} className="">
                    <span className="">
                      {v.from}::{v.name}
                    </span>
                    <span className="pl-1">(</span>
                    {v.argsType().map((a, i) => (
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
                        void v.run()
                      }
                    >
                      Run
                    </button>
                  </li>
                ))}
              </ul>
            </Card>
          </div>
        ))}
      </ResponsiveGridLayout>
    </div>
  );
}
