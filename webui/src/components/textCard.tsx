import { Card } from "./card";
import { Text } from "webcface";
interface Props {
  text: Text[];
}
export function TextCard(props: Props) {
  return (
    <Card title="Text Status">
      <ul className="list-none">
        {props.text.map((v) => (
          <li key={v.name}>
            {v.from}::{v.name} = {v.get()}
          </li>
        ))}
      </ul>
    </Card>
  );
}
