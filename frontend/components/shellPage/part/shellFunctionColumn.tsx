import { useState, useEffect } from "react";
import { Grid, Checkbox, Box } from "@mui/material";

import Name from "../element/name";
import Arguments from "../element/arguments";
import StartButton from "../element/startButton";

import BookmarkBorderIcon from "@mui/icons-material/BookmarkBorder";
import BookmarkIcon from "@mui/icons-material/Bookmark";
import { AnyValue, ArgInfo } from "../lib/global";

type Props = {
  name: string;
  args: ArgInfo[];
  onSubmit: (args: AnyValue[]) => void;
  fav: boolean;
  addFav: (name: string) => void;
  delFav: (name: string) => void;
};

export default function ShellFunctionColumn(props: Props) {
  const [values, setValues] = useState<AnyValue[]>([]);
  useEffect(() => {
    setValues((values) =>
      props.args.map((a, i) => (i < values.length ? values[i] : a.default))
    );
  }, [props.args]);
  const [mouse, setMouse] = useState<boolean>(false);
  const valueOk = props.args.map((a, ai) => {
    switch (a.type) {
      case "bool":
        return typeof values[ai] === "boolean";
      case "int":
      case "float":
        return typeof values[ai] === "number" || !isNaN(Number(values[ai]));
      default:
        return true;
    }
  });
  const convertValue = () => props.args.map((a, ai) => {
    switch(a.type){
     case "bool":
        return Boolean(values[ai]);
      case "int":
      case "float":
        return Number(values[ai]);
      default:
        return values[ai]; 
    }
  })
  return (
    <Box sx={{ background: mouse ? "lightyellow" : "inherit" }}>
      <Grid
        container
        spacing={1}
        alignItems="center"
        onMouseOver={() => setMouse(true)}
        onMouseOut={() => setMouse(false)}
      >
        <Grid item>
          <Checkbox
            icon={<BookmarkBorderIcon />}
            checkedIcon={<BookmarkIcon />}
            checked={props.fav}
            onChange={() => {
              props.fav ? props.delFav() : props.addFav();
            }}
          />
        </Grid>
        <Grid item>
          <Name>{props.name}</Name>
        </Grid>
        <Grid item xs>
          <Arguments
            args={props.args}
            values={values}
            setValues={setValues}
            valueOk={valueOk}
            onSubmit={() => {
              if (!valueOk.includes(false)) {
                props.onSubmit(convertValue());
              }
            }}
          />
        </Grid>
        <Grid item>
          <StartButton
            onClick={() => {
              if (!valueOk.includes(false)) {
                props.onSubmit(convertValue());
              }
            }}
          />
        </Grid>
      </Grid>
    </Box>
  );
}
