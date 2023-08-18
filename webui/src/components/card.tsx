import { ReactNode } from "react";
interface Props {
  title: string;
  children: ReactNode;
}
export function Card(props: Props) {
  return (
    <div
      className={
        "flex flex-col rounded-md border border-neutral-200 " +
        "bg-white w-full h-full shadow-md overflow-hidden"
      }
    >
      <div
        className={
          "flex-none py-1 " +
          "cursor-grab active:cursor-grabbing MyCardHandle " +
          "hover:shadow rounded-t-md "
        }
      >
        <h3 className="text-center font-semibold ">{props.title}</h3>
      </div>
      <div className="flex-1 p-1 min-h-0 ">{props.children}</div>
    </div>
  );
}
