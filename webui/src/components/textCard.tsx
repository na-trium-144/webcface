import { Card } from "./card";
import { Text } from "webcface";
import { useState, useEffect } from "react";

interface Props {
  name: string;
  text: Text[];
}
export function TextCard(props: Props) {
  const [textCurrent, setTextCurrent] = useState<string[]>([]);
  useEffect(() => {
    const onTextChange = () => {
      setTextCurrent(props.text.map((t) => t.get()));
    };
    onTextChange();
    for (const t of props.text) {
      t.on(onTextChange);
    }
    return () => {
      for (const t of props.text) {
        t.off(onTextChange);
      }
    };
  }, [props.text]);
  return (
    <Card title={`${props.name} Text Variables`}>
      <ul className="list-none">
        {props.text.map((t, i) => (
          <li key={t.name}>
            {t.name} = {textCurrent[i]}
          </li>
        ))}
      </ul>
    </Card>
  );
}
