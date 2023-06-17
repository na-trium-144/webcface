import { TextField, MenuItem, InputLabel, FormControl } from "@mui/material";
import Select from "@mui/material/Select";
import { Ref } from "react";
import { AnyValue, ArgInfo } from "../../../lib/global";

type Props<T> = {
  name: string;
  value: T;
  onChange;
  onSubmit;
  inputRef: Ref<any>;
};

export function IntArg(props: Props<number>) {
  return (
    <TextField
      required
      inputRef={props.inputRef}
      label={props.name}
      variant="outlined"
      type="number"
      value={props.value}
      onChange={(e) => {
        props.onChange(e.target.value);
      }}
      onKeyDown={(e) => {
        if (e.key === "Enter") props.onSubmit();
      }}
      size="small"
    />
  );
}

export function BoolArg(props: Props<boolean>) {
  return (
    <FormControl fullWidth>
      <InputLabel id="boolean-selector-label">{props.name}</InputLabel>
      <Select
        labelId="boolean-selector-label"
        id="boolean-selector"
        inputRef={props.inputRef}
        value={props.value ? "True" : "False"}
        label={props.name}
        onChange={(e) => {
          props.onChange(e.target.value === "True");
        }}
        size="small"

        // TODO: ↓を実装
        // onClose={e => {
        //   if (e.nativeEvent instanceof PointerEvent) return
        //   let target = e.target as HTMLElement;
        //   if (props.idx === props.argValues.length - 1) {
        //     const new_args = props.argValues.map((a, j) => (props.idx === j ? target.dataonChange.value === "True" : a));
        //     props.submitFunc(new_args)
        //   }
        // }}
        // onOpen={_ => { props.onSubmit(); }} ←これEnter押したときじゃないじゃん
      >
        <MenuItem value={"True"}> True </MenuItem>
        <MenuItem value={"False"}> False </MenuItem>
      </Select>
    </FormControl>
  );
}

export function AnyArg(props: Props<any>) {
  return (
    <TextField
      required
      inputRef={props.inputRef}
      label={props.name}
      variant="outlined"
      type=""
      value={props.value === null ? "" : props.value} // TODO: JSON.stringify()いらない？
      onChange={(e) => {
        props.onChange(e.target.value);
      }}
      onKeyDown={(e) => {
        if (e.key === "Enter") props.onSubmit();
      }}
      size="small"
    />
  );
}
