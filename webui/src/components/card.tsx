import { ReactNode } from "react";
interface Props {
  title: string;
  children: ReactNode;
}
export function Card(props: Props) {
  return (
    <div className="flex flex-col rounded-md border border-neutral-200 shadow-md p-1 bg-white w-full h-full">
      <h6 className="flex-none mx-auto font-semibold">{props.title}</h6>
      <div className="flex-auto">{props.children}</div>
    </div>
  );
}
