import { Card } from "./card";
import { Member, Logs, LogLine } from "webcface";
import { useState, useEffect } from "react";
import { format } from "date-fns";

interface Props {
  member: Member;
}
const levelNames = ["Trace", "Debug", "Info", "Warn", "Error", "Critical"];
const levelColors = [
  "text-inherit ",
  "text-cyan-600 ",
  "text-green-600 ",
  "text-amber-600 ",
  "text-red-600 ",
  "text-white bg-red-600 ",
];
export function LogCard(props: Props) {
  const [logsCurrent, setLogsCurrent] = useState<LogLine[]>([]);
  const [minLevel, setMinLevel] = useState<number>(2);
  useEffect(() => {
    const update = () => setLogsCurrent(props.member.log().get().filter((l) => l.level >= minLevel));
    update();
    props.member.log().on(update);
    return () => {
      props.member.log().off(update);
    };
  }, [props.member, minLevel]);

  return (
    <Card title={`${props.member.name} Logs`}>
      <div className="flex flex-col w-full h-full">
        <div className="flex-none pl-2 pb-1">
          レベル
          <input
            className={
              "border-0 border-b outline-0 mx-1 w-12 text-center " +
              "border-neutral-200 hover:border-neutral-500 focus:border-black "
            }
            type="number"
            value={minLevel}
            onChange={(e) => {
              setMinLevel(e.target.value);
            }}
          />
          以上のログを表示: 全<span className="px-1">{props.member.log().get().length}</span>
          行中
          <span className="px-1">{logsCurrent.length}</span>行
        </div>
        <table className="flex-1 block table-auto overflow-auto text-sm">
          <thead>
            <tr className="border-b">
              <th className="font-medium">Time</th>
              <th className="font-medium">Level</th>
              <th className="font-medium">Message</th>
            </tr>
          </thead>
          <tbody>
            {logsCurrent.map((l, li) => (
              <tr key={li} className="border-b">
                <td className="text-center">{format(l.time, "H:mm:ss.SSS")}</td>
                <td className="text-center">
                  <span
                    className={
                      "px-0.5 rounded-sm " + (levelColors[l.level] || "")
                    }
                  >
                    {l.level}
                    {levelNames[l.level] !== ""
                      ? `(${levelNames[l.level]})`
                      : ""}
                  </span>
                </td>
                <td className="px-1 font-mono w-full">{l.message}</td>
              </tr>
            ))}
            <tr className="text-transparent select-none">
              <td className="px-1 text-center">0:00:00.000</td>
              <td className="px-1.5 text-center">5(Critical)</td>
              <td className="px-1 font-mono w-full"></td>
            </tr>
          </tbody>
        </table>
      </div>
    </Card>
  );
}
