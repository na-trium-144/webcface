import * as React from "react";
import { Divider, Stack, TextareaAutosize, Typography } from "@mui/material";

type Props = {
  title: string;
  data: string[];
};

export default function TextLog(props: Props) {
  return (
    <Stack spacing={1}>
      <Typography variant="body1">
        {props.data != undefined && (typeof props.data === "string" ? props.data : props.data.toString())}
      </Typography>
      <Divider />
      {/*<TextareaAutosize maxRows={10} defaultValue={props.data.reverse().join("\n")} />*/}
    </Stack>
  );
}
