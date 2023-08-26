import { Card } from "./card";
import { useForceUpdate } from "../libs/forceUpdate";
import { Member, View, ViewComponent, viewComponentTypes } from "webcface";
import { useState, useEffect } from "react";

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
function ViewComponentRender(props: VCProps) {
  switch (props.vc.type) {
    case viewComponentTypes.text:
      return <span>{props.vc.text}</span>;
    case viewComponentTypes.newLine:
      return <br />;
  }
}
