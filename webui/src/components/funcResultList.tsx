import { useFuncResult } from "./funcResult";
import { useState, useEffect } from "react";

interface ResultDisplay {
  member: string;
  name: string;
  status: number;
  result: string;
  show: boolean;
}
const funcStatus = {
  connecting: 0,
  running: 1,
  ok: 2,
  error: 3,
};

export function FuncResultList() {
  const { results } = useFuncResult();
  const [resultsDisplay, setResultsDisplay] = useState<ResultDisplay[]>([]);
  useEffect(() => {
    if (resultsDisplay.length < results.length) {
      const resultsDisplayNew = resultsDisplay.slice();
      for (let i = resultsDisplay.length; i < results.length; i++) {
        resultsDisplayNew.push({
          // todo: getterないの?
          member: results[i].member_,
          name: results[i].field_,
          status: funcStatus.connecting,
          result: "",
          show: true,
        });
        setTimeout(() => {
          void results[i].started.then(() =>
            setResultsDisplay((resultsDisplay) =>
              resultsDisplay.map((d, j) =>
                i === j && d.status === funcStatus.connecting
                  ? { ...d, status: funcStatus.running }
                  : d
              )
            )
          );
        });
        const closeResult = () => {
          setResultsDisplay((resultsDisplay) =>
            resultsDisplay.map((d, j) => (i === j ? { ...d, show: false } : d))
          );
        };
        setTimeout(() => {
          void results[i].result
            .then((val: string | number | boolean | null) => {
              setResultsDisplay((resultsDisplay) =>
                resultsDisplay.map((d, j) =>
                  i === j
                    ? { ...d, result: String(val), status: funcStatus.ok }
                    : d
                )
              );
              setTimeout(closeResult, 5000);
            })
            .catch((e) => {
              setResultsDisplay((resultsDisplay) =>
                resultsDisplay.map((d, j) =>
                  i === j
                    ? {
                        ...d,
                        result: (e as Error).toString(),
                        status: funcStatus.error,
                      }
                    : d
                )
              );
              setTimeout(closeResult, 30000);
            });
        });
      }
      setResultsDisplay(resultsDisplayNew);
    }
  }, [results, resultsDisplay]);

  console.log(results);
  console.log(resultsDisplay);
  const listShow = resultsDisplay.filter((d) => d.show).length > 0;

  return (
    <div
      className={
        "fixed right-2 bottom-2 w-72 max-h-[25%] p-2 " +
        "rounded-lg shadow-lg overflow-x-hidden overflow-y-auto bg-white " +
        "transition duration-100 origin-bottom-right " +
        (listShow
          ? "ease-out opacity-100 scale-100 z-[999] "
          : "ease-in opacity-0 scale-90 -z-10 ")
      }
    >
      <ul>
        {resultsDisplay.map(
          (d, i) =>
            d.show && (
              <li key={i}>
                <span className="text-xs pr-1">{d.member}</span>
                <span className="text-sm pr-1">{d.name}</span>
                {
                  [
                    <span className="text-sm text-blue-500 ">
                      Connecting...
                    </span>,
                    <span className="text-sm text-green-500 ">Running...</span>,
                    <>
                      <span className="text-sm pr-1">ok</span>
                      <span className="text-sm font-mono">{d.result}</span>
                    </>,
                    <>
                      <span className="text-red-500 text-sm pr-1">Error</span>
                      <span className="text-red-500 text-sm font-mono">
                        {d.result}
                      </span>
                    </>,
                  ][d.status]
                }
              </li>
            )
        )}
      </ul>
    </div>
  );
}
