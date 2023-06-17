import { useState, useEffect, Fragment } from "react";
import { Grid, Stack, Box, Divider, Typography } from "@mui/material";
import { RobotCallbackList } from "../lib/global";
import BookmarkIcon from "@mui/icons-material/Bookmark";

import ShellFunctionColumn from "../components/shellPage/part/shellFunctionColumn";
import { useSocket } from "../components/socketContext";
import { useFunctionFavs, FunctionFavs } from "../lib/useFunctionFavs";
import { AnyValue, FunctionSettingT, MultiSocketContextI } from "../lib/global";
import ServerSelectTab from "../components/serverSelectTab";

const looseJsonParse = (obj: AnyValue) => {
  try {
    return Function('"use strict";return (' + obj + ")")() as AnyValue;
  } catch {
    return obj;
  }
};

const call =
  (
    socket: MultiSocketContextI,
    serverIndex: number,
    fp: FunctionSettingT,
  ) =>
  (new_args: AnyValue[]) => {
    console.log("Run Function!!  ", fp.name, new_args);
    const newargs_send = {};
    for (let i = 0; i < new_args.length; i++) {
      newargs_send[fp.args[i].name] = looseJsonParse(new_args[i]);
    }
    socket.getByIndex(serverIndex).runCallback(fp.name, newargs_send);
  };

export const ShellPageFav = (props: {
  favs: FunctionFavs;
}) => {
  const socket = useSocket();
  const { addFav, delFav, isFav } = props.favs;

  return (
    <Stack spacing={0}>
      {socket.raw.reduce(
        (prev, s, si) =>
          prev.concat(
            s.functionSetting
              .filter((fp) => isFav(`${s.serverName}:${fp.name}`))
              .map((fp, idx) => (
                <ShellFunctionColumn
                  key={`${si}:${idx}`}
                  name={`${s.serverName}:${fp.name}`}
                  args={fp.args}
                  onSubmit={call(socket, si, fp)}
                  fav={isFav(`${s.serverName}:${fp.name}`)}
                  addFav={() => addFav(`${s.serverName}:${fp.name}`)}
                  delFav={() => delFav(`${s.serverName}:${fp.name}`)}
                />
              ))
          ),
        []
      )}
    </Stack>
  );
};
export const ShellPageServer = (props: {
  sid: number;
  favs: FunctionFavs;
}) => {
  const socket = useSocket();
  const { addFav, delFav, isFav } = props.favs;

  return (
    <Stack spacing={0}>
      {socket.getByIndex(props.sid).functionSetting.map((fp, idx) => (
        <ShellFunctionColumn
          key={idx}
          name={fp.name}
          args={fp.args}
          onSubmit={call(socket, props.sid, fp)}
          fav={isFav(`${socket.getByIndex(props.sid).serverName}:${fp.name}`)}
          addFav={() =>
            addFav(`${socket.getByIndex(props.sid).serverName}:${fp.name}`)
          }
          delFav={() =>
            delFav(`${socket.getByIndex(props.sid).serverName}:${fp.name}`)
          }
        />
      ))}
    </Stack>
  );
};
export default function ShellPage() {
  const socket = useSocket();
  const [tabPage, setTabPage] = useState<number>(0);

  const favs = useFunctionFavs();

  return (
    <Fragment>
      <Box>
        <BookmarkIcon sx={{ fontSize: 18 }} color="primary" />
        <Typography variant="caption" gutterBottom>
          を押したものが上に表示されます
        </Typography>
      </Box>
      <ShellPageFav favs={favs} />
      <ServerSelectTab
        tabPage={tabPage}
        setTabPage={setTabPage}
        serverNames={socket.raw.map((s) => s.serverName)}
      />
      <ShellPageServer sid={tabPage} favs={favs} />
    </Fragment>
  );
}
