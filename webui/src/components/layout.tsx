import { useState, useEffect, useRef } from "react";
import { Client, Value, Text, Func } from "webcface";
import "../index.css";
import "react-grid-layout/css/styles.css";
import "react-resizable/css/styles.css";
import { Responsive, WidthProvider, Layout, Layouts } from "react-grid-layout";
const ResponsiveGridLayout = WidthProvider(Responsive);
import { getFromLS, saveToLS } from "../libs/ls";
import { Card, CardItem } from "./card";
import { ValueCard } from "./valueCard";
import { TextCard } from "./textCard";
import { FuncCard } from "./funcCard";
import { MemberValues, MemberTexts, MemberFuncs } from "../libs/stateTypes";
import * as cardKey from "../libs/cardKey";

interface Props {
  isOpened: (key: string) => boolean;
  openedOrder: (key: string) => number;
  moveOrder: (key: string) => void;
  memberValues: MemberValues[];
  memberTexts: MemberTexts[];
  memberFuncs: MemberFuncs[];
}

export function LayoutMain(props: Props) {
  const [layouts, setLayouts] = useState<Layouts>({});
  const [lsLayout, setLsLayout] = useState<Layout[]>([]);
  const currentLayout = useRef<Layout[]>([]);
  const [layoutsLoadDone, setLayoutsLoadDone] = useState<boolean>(false);
  const prevLayout = useRef<Layout[]>([]);

  const breakpoints = {
    xxl: 1536,
    xl: 1280,
    lg: 1024,
    md: 768,
    sm: 640,
    xs: 0,
  };
  const cols = { xxl: 15, xl: 13, lg: 10, md: 7, sm: 6, xs: 2 };
  const layoutsAll = (layout: Layout[]) => {
    return Object.keys(breakpoints).reduce((obj, k) => {
      obj[k] = layout.map((l) => ({ ...l }));
      return obj;
    }, {});
  };
  const isLayoutSame = (l1: Layout, l2: Layout) => {
    if (l1.x !== l2.x) return false;
    if (l1.y !== l2.y) return false;
    if (l1.w !== l2.w) return false;
    if (l1.h !== l2.h) return false;
    return true;
  };

  useEffect(() => {
    const ls = getFromLS("layout") || [];
    setLsLayout(ls);
    setLayouts(layoutsAll(ls));
    setLayoutsLoadDone(true);
  }, []);

  // https://github.com/react-grid-layout/react-grid-layout/issues/1775
  // onLayoutChangeが正しく呼ばれないバグあり、
  // 代わりにlayoutを参照で保持し変更されているかこっちでチェックする
  const onLayoutChange = (layout: Layout[], layouts: Layouts) => {
    currentLayout.current = layout;
    // console.log("layoutchange", layout)
  };
  useEffect(() => {
    const i = setInterval(() => {
      let changed = false;
      for (let nli = 0; nli < currentLayout.current.length; ++nli) {
        const nl = currentLayout.current[nli];
        const pli = prevLayout.current.findIndex((pl) => pl.i === nl.i);
        const lli = lsLayout.findIndex((ll) => ll.i === nl.i);
        if (pli >= 0) {
          if (!isLayoutSame(prevLayout.current[pli], nl)) {
            prevLayout.current[pli] = { ...nl };
            changed = true;
          }
        } else {
          changed = true;
          if (lli >= 0) {
            currentLayout.current[nli] = lsLayout[lli];
          }
          prevLayout.current.push({ ...currentLayout.current[nli] });
        }
      }
      if (changed && layoutsLoadDone) {
        saveToLS("layout", currentLayout.current);
        setLsLayout(currentLayout.current.map((nl) => ({ ...nl })));
        setLayouts(layoutsAll(currentLayout.current));
      }
    }, 100);
    return () => clearInterval(i);
  }, [layoutsLoadDone, lsLayout, setLsLayout, setLayouts]);

  return (
    <ResponsiveGridLayout
      className="layout"
      layouts={layouts}
      breakpoints={breakpoints}
      cols={cols}
      rowHeight={100}
      onLayoutChange={onLayoutChange}
      allowOverlap
      compactType={null}
      draggableHandle=".MyCardHandle"
    >
      {props.memberValues
        .reduce((prev, m) => prev.concat(m.values), [] as Value[])
        .map((v) => {
          const key = cardKey.value(v.member.name, v.name);
          if (key) {
            return (
              <div
                key={key}
                data-grid={{ x: 0, y: 0, w: 2, h: 2, minW: 2, minH: 2 }}
                style={{
                  zIndex: 10 + props.openedOrder(key),
                }}
                onPointerDown={() => props.moveOrder(key)}
              >
                <ValueCard value={v} />
              </div>
            );
          }
          return null;
        })}
      {props.memberTexts.map((m) => {
        const key = cardKey.text(m.name);
        if (props.isOpened(key)) {
          return (
            <div
              key={key}
              data-grid={{ x: 0, y: 0, w: 4, h: 2, minW: 2, minH: 1 }}
              style={{ zIndex: 10 + props.openedOrder(key) }}
              onPointerDown={() => props.moveOrder(key)}
            >
              <TextCard name={m.name} text={m.texts} />
            </div>
          );
        }
        return null;
      })}
      {props.memberFuncs.map((m) => {
        const key = cardKey.func(m.name);
        if (props.isOpened(key)) {
          return (
            <div
              key={key}
              data-grid={{ x: 0, y: 0, w: 6, h: 2, minW: 2, minH: 2 }}
              style={{ zIndex: 10 + props.openedOrder(key) }}
              onPointerDown={() => props.moveOrder(key)}
            >
              <FuncCard name={m.name} func={m.funcs} />
            </div>
          );
        }
        return null;
      })}
    </ResponsiveGridLayout>
  );
}
