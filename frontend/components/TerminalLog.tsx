import { TerminalLogT } from "@/lib/global";
import {
  Table,
  Typography,
  TableBody,
  TableCell,
  TableHead,
  TableRow,
} from "@mui/material";
import { useState, useEffect, useRef } from "react";
import * as React from "react";
import { useSocket } from "../components/socketContext";
import ServerSelectTab from "../components/serverSelectTab";

const format_date = (d: Date) => {
  let result = "";
  result +=
    " " +
    d.getHours() +
    ":" +
    d.getMinutes() +
    ":" +
    d.getSeconds() +
    "." +
    d.getMilliseconds();
  return result;
};

export const TerminalLogTable = (props: { sid: number }) => {
  const socket = useSocket();
  const [terminalLog, setTerminalLog] = useState<TerminalLogT[]>([]);
  const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  useEffect(() => {
    setTerminalLog(socket.getByIndex(props.sid).getTerminalLog());
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 100);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag, props.sid, socket]);

  return (
    <>
      <Typography variant="body1">{terminalLog.length} Logs</Typography>
      <Table size="small">
        <TableHead>
          <TableRow>
            <TableCell>timestamp</TableCell>
            <TableCell>level</TableCell>
            <TableCell align="left">message</TableCell>
          </TableRow>
        </TableHead>
        <TableBody>
          {terminalLog.map((log: TerminalLogT, index) => (
            <TableRow key={index}>
              <TableCell component="th" scope="row">
                {format_date(log.timestamp)}
              </TableCell>
              <TableCell align="right">{log.level}</TableCell>
              <TableCell align="left">{log.message}</TableCell>
            </TableRow>
          ))}
        </TableBody>
      </Table>
    </>
  );
};
export default function TerminalLog() {
  const socket = useSocket();
  const [tabPage, setTabPage] = useState<number>(0);

  return (
    <>
      <ServerSelectTab
        tabPage={tabPage}
        setTabPage={setTabPage}
        serverNames={socket.raw.map((s) => s.serverName)}
      />
      <TerminalLogTable sid={tabPage} />
    </>
  );
}
