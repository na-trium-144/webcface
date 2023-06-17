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
    setValues(props.args.map((a) => a.default));
  }, [props.args]);
  const [mouse, setMouse] = useState<boolean>(false);
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
            onSubmit={() => {
              props.onSubmit(values);
            }}
          />
        </Grid>
        <Grid item>
          <StartButton
            onClick={() => {
              props.onSubmit(values);
            }}
          />
        </Grid>
      </Grid>
    </Box>
  );
}
