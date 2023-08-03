import { useState, useEffect } from "react";
import { Card } from "./card";
import { Func, argType } from "webcface";

interface Props {
  func: Func[];
}
export function FuncCard(props: Props) {
  return (
    <Card title="Functions">
      <ul className="list-none">
        {props.func.map((v) => (
          <li key={`${v.from}::${v.name}`} className="">
            <FuncLine func={v} />
          </li>
        ))}
      </ul>
    </Card>
  );
}

interface ArgProps {
  type: number;
  arg: string | number | boolean;
  setArg: (arg: string | number | boolean) => void;
  isError: boolean;
  setIsError: (isError: boolean) => void;
  id: string;
}
function ArgInput(props: ArgProps) {
  const [inputClass, setInputClass] = useState<string>("");
  useEffect(() => {
    setInputClass(
      "border-0 border-b outline-0 focus:border-b-2 px-1 " +
        (props.isError
          ? "border-red-500 hover:border-red-500 focus:border-red-500 "
          : "border-neutral-200 hover:border-neutral-500 focus:border-black ")
    );
  }, [props.isError]);

  switch (props.type) {
    case argType.int_:
      return (
        <input
          type="number"
          className={inputClass + " w-20"}
          value={props.arg}
          onChange={(e) => props.setArg(e.target.value)}
        />
      );
    case argType.boolean_:
      return (
        <button
          type="button"
          checked={props.arg}
          onClick={() => props.setArg(!props.arg)}
          className={
            inputClass +
            "cursor-pointer inline-block pl-1 w-12 \
              hover:text-green-700 active:text-green-700 "
          }
        >
          {props.arg ? "true" : "false"}
        </button>
      );
    case argType.float_:
      return (
        <input
          type="text"
          className={inputClass}
          size="6"
          value={String(props.arg)}
          onChange={(e) => {
            if (isNaN(Number(e.target.value))) {
              props.setIsError(true);
              props.setArg(e.target.value);
            } else {
              props.setIsError(false);
              props.setArg(e.target.value);
            }
          }}
        />
      );
    default:
      return (
        <>
          <span>"</span>
          <input
            type="text"
            className={inputClass}
            size="6"
            value={props.arg}
            onChange={(e) => props.setArg(e.target.value)}
          />
          <span className="pr-1">"</span>
        </>
      );
  }
}
function FuncLine(props: { func: Func }) {
  const [args, setArgs] = useState<(string | number | boolean)[]>([]);
  const [errors, setErrors] = useState<boolean[]>([]);
  useEffect(() => {
    if (args.length < props.func.argsType().length) {
      setArgs(
        props.func.argsType().map((at, i) => {
          if (i < args.length) {
            return args[i];
          } else {
            switch (at) {
              case argType.int_:
              case argType.float_:
                return 0;
              case argType.boolean_:
                return false;
              default:
                return "";
            }
          }
        })
      );
      setErrors(
        props.func.argsType().map((at, i) => {
          if (i < args.length) {
            return errors[i];
          } else {
            return false;
          }
        })
      );
    }
  }, [props.func, args, setArgs, errors, setErrors]);
  return (
    <>
      <span className="">
        {props.func.from}::{props.func.name}
      </span>
      <span className="pl-1">(</span>
      {props.func.argsType().map((at, i) => (
        <span key={i}>
          {i > 0 && <span className="pl-1 pr-1">,</span>}
          <ArgInput
            type={at}
            arg={args[i]}
            setArg={(arg) =>
              setArgs(args.map((ca, ci) => (i === ci ? arg : ca)))
            }
            isError={errors[i]}
            setIsError={(isError) =>
              setErrors(errors.map((ce, ci) => (i === ci ? isError : ce)))
            }
            id={`${props.func.from}-func-${props.func.name}-arg-${i}`}
          />
        </span>
      ))}
      <span className="pr-2">)</span>
      <button
        className={
          "rounded-full border px-2 my-1 " +
          (errors.includes(true)
            ? "border-neutral-400 bg-neutral-300 "
            : "shadow-md border-green-300 bg-green-100 hover:bg-green-200 \
              active:shadow-none active:bg-green-300 ")
        }
        disabled={errors.includes(true)}
        onClick={() => {
          // todo: 引数
          console.log(args);
          void props.func.run(...args);
        }}
      >
        Run
      </button>
    </>
  );
}
