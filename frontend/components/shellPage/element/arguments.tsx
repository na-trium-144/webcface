import { createRef, useRef } from "react";
import { Grid } from "@mui/material";
import { IntArg, BoolArg, AnyArg } from "./inputBox";
import { AnyValue, ArgInfo } from "../../../lib/global";

type Props = {
  args: ReadonlyArray<ArgInfo>;
  values: AnyValue[];
  setValues: (values: AnyValue[]) => void;
  onSubmit: (values: AnyValue[]) => void;
};

/**
 *
 * @param {Props} props
 */
export default function Arguments(props: Props) {
  const inputRefs = useRef([]);
  inputRefs.current = props.args.map(
    (_, i) => inputRefs.current[i] ?? createRef()
  );

  return (
    <Grid container spacing={0.5} justifyContent="flex-end">
      {props.args.map((arg, i) => {
        const value = props.values[i];
        const { name, type } = arg;
        const isEnd = props.args.length - 1 === i;
        const onChange = (new_value: AnyValue) => {
          const new_args = props.values.map((a, j) =>
            i === j ? new_value : a
          );
          props.setValues(new_args);
        };

        const onSubmit = () => {
          if (isEnd) {
            props.onSubmit();
          } else {
            inputRefs.current[i + 1].current.focus();
          }
        };
        const input = (() => {
          switch (type) {
            case "int":
            case "double":
              return (
                <IntArg
                  name={name}
                  value={value}
                  onChange={onChange}
                  onSubmit={onSubmit}
                  inputRef={inputRefs.current[i]}
                />
              );
            case "bool":
              return (
                <BoolArg
                  name={name}
                  value={value}
                  onChange={onChange}
                  onSubmit={onSubmit}
                  inputRef={inputRefs.current[i]}
                />
              );

            default:
              return (
                <AnyArg
                  name={name}
                  value={value}
                  onChange={onChange}
                  onSubmit={onSubmit}
                  inputRef={inputRefs.current[i]}
                />
              );
          }
        })();

        return (
          <Grid item key={i} sx={{width: 150}}>
            {input}
          </Grid>
        );
      })}
    </Grid>
  );
}
