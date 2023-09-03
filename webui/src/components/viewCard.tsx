import { Card } from "./card";
import { useForceUpdate } from "../libs/forceUpdate";
import {
  Member,
  View,
  ViewComponent,
  viewColor,
  viewComponentTypes,
} from "webcface";
import { useState, useEffect } from "react";
import { useFuncResult } from "./funcResult";

interface Props {
  view: View;
}
export function ViewCard(props: Props) {
  const update = useForceUpdate();
  useEffect(() => {
    props.view.on(update);
    return () => props.view.off(update);
  }, [props.view, update]);
  return (
    <Card title={`${props.view.member.name}:${props.view.name}`}>
      <div className="w-full h-full overflow-y-auto overflow-x-auto">
        {props.view.get().map((vc, i) => (
          <ViewComponentRender key={i} vc={vc} />
        ))}
      </div>
    </Card>
  );
}

interface VCProps {
  vc: ViewComponent;
}
const colorName = [
  "inherit",
  "black",
  "white",
  "slate",
  "grey",
  "zinc",
  "neutral",
  "stone",
  "red",
  "orange",
  "amber",
  "yellow",
  "lime",
  "green",
  "emerald",
  "teal",
  "cyan",
  "sky",
  "blue",
  "indigo",
  "violet",
  "purple",
  "fuchsia",
  "pink",
  "rose",
];
const colorClass = (c: number, level = 3, inherit = "-inherit ") => {
  switch (c) {
    case viewColor.inherit:
      return inherit;
    case viewColor.black:
      return level < 3 ? `neutral-${level + 6}00 ` : "black ";
    case viewColor.white:
      return level > 1 ? `neutral-${level - 1}00 ` : "white ";
    default:
      return `${colorName[c]}-${level}00 `;
  }
};
function ViewComponentRender(props: VCProps) {
  const { addResult } = useFuncResult();
  switch (props.vc.type) {
    case viewComponentTypes.text:
      return <span>{props.vc.text}</span>;
    case viewComponentTypes.newLine:
      return <br />;
    case viewComponentTypes.button:
      return (
        <button
          className={
            "rounded-md border px-2 shadow-md active:shadow-none " +
            "border-" +
            colorClass(props.vc.bgColor, 3, "green-300 ") +
            "bg-" +
            colorClass(props.vc.bgColor, 1, "green-100 ") +
            "hover:bg-" +
            colorClass(props.vc.bgColor, 2, "green-200 ") +
            "active:bg-" +
            colorClass(props.vc.bgColor, 3, "green-300 ") +
            "text-" +
            colorClass(props.vc.textColor, 5)
          }
          onClick={() => {
            const r = props.vc.onClick?.runAsync();
            if (r != null) {
              addResult(r);
            }
          }}
        >
          {props.vc.text}
        </button>
      );
  }
}
