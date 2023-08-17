import { Card } from "./card";
import { Value } from "webcface";
import { useState, useEffect } from "react";

interface Props {
  value: Value;
}
export function ValueCard(props: Props) {
  const [valueCurrent, setValueCurrent] = useState<number>(0);
  useEffect(() => {
    const onValueChange = () => setValueCurrent(props.value.get());
    onValueChange();
    props.value.on(onValueChange);
    return () => props.value.off(onValueChange);
  }, [props.value]);
  return (
    <Card title={`${props.value.member.name}:${props.value.name}`}>
      {valueCurrent}
    </Card>
  );
}
