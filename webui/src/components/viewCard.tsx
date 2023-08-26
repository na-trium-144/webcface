import { Card } from "./card";
import { useForceUpdate } from "../libs/forceUpdate";
import { Member, View, ViewComponent, viewComponentsType } from "webcface";
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
      
    </Card>
  );
}

interface VCProps{
  vc: ViewComponent;
}
function ViewComponentRender(props: VCProps){

}