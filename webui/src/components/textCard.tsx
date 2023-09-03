import { Card } from "./card";
import { useForceUpdate } from "../libs/forceUpdate";
import { Member, Text } from "webcface";
import { useState, useEffect } from "react";

interface Props {
  member: Member;
}
export function TextCard(props: Props) {
  const update = useForceUpdate();
  useEffect(() => {
    const onTextEntry = (t: Text) => {
      t.on(update);
      update();
    };
    props.member.textsChange.on(onTextEntry);
    return () => {
      props.member.textsChange.off(onTextEntry);
    };
  }, [props.member, update]);
  return (
    <Card title={`${props.member.name} Text Variables`}>
      <ul className="list-none">
        {props.member.texts().map((t) => (
          <li key={t.name}>
            {t.name} = {t.get()}
          </li>
        ))}
      </ul>
    </Card>
  );
}
