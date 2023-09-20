import { useState, useEffect, useRef } from "react";
import { Card } from "./card";
import { useForceUpdate } from "../libs/forceUpdate";
import { Member, Func, Arg, valType } from "webcface";
import { useFuncResult } from "./funcResult";

interface Props {
  member: Member;
}
export function FuncCard(props: Props) {
  const update = useForceUpdate();
  useEffect(() => {
    props.member.onFuncEntry.on(update);
    return () => {
      props.member.onFuncEntry.off(update);
    };
  }, [props.member, update]);
  return (
    <Card title={`${props.member.name} Functions`}>
      <ul className="list-none">
        {props.member.funcs().map((v) => (
          <li key={v.name}>
            <FuncLine func={v} />
          </li>
        ))}
      </ul>
    </Card>
  );
}

interface ArgProps {
  argConfig: Arg;
  arg: string | number | boolean;
  setArg: (arg: string | number | boolean) => void;
  isError: boolean;
  setIsError: (isError: boolean) => void;
  id: string;
}
function ArgInput(props: ArgProps) {
  const inputClass = "border-0 outline-0 px-1 peer ";
  const checkError = useRef<(() => void) | null>(null);
  useEffect(() => {
    if (
      checkError.current != null &&
      !props.isError &&
      checkError.current(String(props.arg))
    ) {
      props.setIsError(true);
    }
  }, [props.arg]);

  if (props.argConfig.option.length > 0) {
    return (
      <select
        className={inputClass + "px-0 "}
        value={props.arg}
        onChange={(e) => props.setArg(e.target.value)}
      >
        {props.argConfig.option.map((o, oi) => (
          <option key={oi} value={String(o)}>
            {String(o)}
          </option>
        ))}
      </select>
    );
  } else {
    switch (props.argConfig.type) {
      case valType.int_:
        return (
          <input
            type="number"
            className={inputClass + "w-20"}
            value={(props.arg as number) || 0}
            min={props.argConfig.min}
            max={props.argConfig.max}
            onChange={(e) => {
              props.setIsError(!e.target.checkValidity());
              props.setArg(e.target.value);
            }}
          />
        );
      case valType.boolean_:
        return (
          <button
            type="button"
            checked={!!props.arg}
            onClick={() => props.setArg(!props.arg)}
            className={
              inputClass +
              "cursor-pointer inline-block pl-1 w-12 " +
              "hover:text-green-700 active:text-green-700 "
            }
          >
            {props.arg ? "true" : "false"}
          </button>
        );
      case valType.float_: {
        checkError.current = (v: string) =>
          isNaN(Number(v)) ||
          (props.argConfig.min != null && props.argConfig.min > Number(v)) ||
          (props.argConfig.max != null && props.argConfig.max < Number(v));
        return (
          <input
            type="text"
            className={inputClass}
            size={6}
            value={String(props.arg)}
            onChange={(e) => {
              props.setIsError(checkError.current(e.target.value));
              props.setArg(e.target.value);
            }}
          />
        );
      }
      default: {
        checkError.current = (v: string) =>
          (props.argConfig.min != null && props.argConfig.min > v.length) ||
          (props.argConfig.max != null && props.argConfig.max < v.length);
        return (
          <input
            type="text"
            className={inputClass}
            size={6}
            value={String(props.arg)}
            onChange={(e) => {
              props.setIsError(checkError.current(e.target.value));
              props.setArg(e.target.value);
            }}
          />
        );
      }
    }
  }
}

function FuncLine(props: { func: Func }) {
  const [args, setArgs] = useState<(string | number | boolean)[]>([]);
  const [errors, setErrors] = useState<boolean[]>([]);
  const { addResult } = useFuncResult();
  useEffect(() => {
    if (args.length < props.func.args.length) {
      setArgs(
        props.func.args.map((ac, i) => {
          if (i < args.length) {
            return args[i];
          } else if (ac.init != null) {
            return ac.init;
          } else {
            switch (ac.type) {
              case valType.int_:
              case valType.float_:
                return 0;
              case valType.boolean_:
                return false;
              default:
                return "";
            }
          }
        })
      );
      setErrors(
        props.func.args.map((ac, i) => {
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
      <span>{props.func.name}</span>
      <span className="pl-1 pr-0.5">(</span>
      <span>
        {props.func.args.map((ac, i) => (
          <>
            <span key={`sep-${i}`} className="pl-1 pr-1 first:hidden">
              ,
            </span>
            {ac.type === valType.string_ && <span>"</span>}
            <div key={i} className="inline-block relative pt-3">
              <ArgInput
                argConfig={ac}
                arg={args[i]}
                setArg={(arg) =>
                  setArgs(args.map((ca, ci) => (i === ci ? arg : ca)))
                }
                isError={errors[i]}
                setIsError={(isError) =>
                  setErrors(errors.map((ce, ci) => (i === ci ? isError : ce)))
                }
                id={`${props.func.member.name}-func-${props.func.name}-arg-${i}`}
              />
              <span
                className={
                  "absolute bottom-0 inset-x-0 " +
                  "border-b peer-focus:border-b-2 px-1 peer " +
                  (errors[i]
                    ? "border-red-500 peer-hover:border-red-500 peer-focus:border-red-500 "
                    : "border-neutral-200 peer-hover:border-neutral-500 peer-focus:border-black ")
                }
              />
              <span
                className={
                  "absolute top-0 left-0.5 text-xs " +
                  "text-neutral-400 peer-focus:text-black "
                }
              >
                {ac.name}
              </span>
              <ArgDescription argConfig={ac} />
            </div>
            {ac.type === valType.string_ && <span>"</span>}
          </>
        ))}
      </span>
      <span className="pl-0.5 pr-2">)</span>
      <button
        className={
          "rounded-full border px-2 my-1 " +
          (errors.includes(true)
            ? "border-neutral-400 bg-neutral-300 "
            : "shadow-md border-green-300 bg-green-100 hover:bg-green-200 \
              active:shadow-none active:bg-green-300 ")
        }
        disabled={errors.includes(true)}
        onClick={() => addResult(props.func.runAsync(...args))}
      >
        Run
      </button>
    </>
  );
}

function ArgDescription(props: { argConfig: Arg }) {
  const valTypeText = () => {
    switch (props.argConfig.type) {
      case valType.int_:
        return "整数型";
      case valType.float_:
        return "実数型";
      case valType.boolean_:
        return "真偽値型";
      case valType.string_:
        return "文字列型";
      default:
        return "";
    }
  };
  return (
    <div
      className={
        "absolute top-full left-1/2 -translate-x-2/4 translate-y-2 text-center opacity-90 " +
        "hidden peer-focus:inline-block peer-hover:inline-block " +
        "bg-green-900 p-1 text-white text-xs rounded min-w-max "
      }
    >
      <span
        className={
          "absolute top-0 left-1/2 -translate-x-2/4 -translate-y-2 " +
          "border-4 border-transparent border-b-green-900"
        }
      />
      <div>
        {valTypeText()}
        {props.argConfig.option.length > 0 && " (選択式)"}
      </div>
      <div>
        {props.argConfig.min != null &&
          (props.argConfig.type === valTypeText.string_
            ? `最小長さ ${props.argConfig.min}`
            : `最小値 ${props.argConfig.min}`)}
        {props.argConfig.min != null && props.argConfig.max != null && ", "}
        {props.argConfig.max != null &&
          (props.argConfig.type === valTypeText.string_
            ? `最大長さ ${props.argConfig.max}`
            : `最大値 ${props.argConfig.max}`)}
      </div>
      <div>
        {props.argConfig.init != null &&
          `初期値 ${String(props.argConfig.init)}`}
      </div>
    </div>
  );
}
