import { useContext, createContext, useState, ReactElement } from "react";
import { AsyncFuncResult } from "webcface";

interface FuncResult {
  results: AsyncFuncResult[];
  addResult: (result: AsyncFuncResult) => void;
}
const FuncResultContext = createContext<FuncResult>({
  results: [],
  addResult: () => undefined,
});
export const useFuncResult = () => useContext(FuncResultContext);

export function FuncResultProvider(props: { children: ReactElement }) {
  const [results, setResults] = useState<AsyncFuncResult[]>([]);
  console.log("context update")
  return (
    <FuncResultContext.Provider
      value={{
        results,
        addResult: (result: AsyncFuncResult) =>
          setResults(results.concat([result])),
      }}
    >
      {props.children}
    </FuncResultContext.Provider>
  );
}
