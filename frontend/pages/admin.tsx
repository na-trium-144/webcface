import { useSocket } from "../components/socketContext";
import ServerSelectTab from "../components/serverSelectTab";
import { useState, useEffect, Fragment } from "react";
import "@rich-data/viewer/theme/base.css";
import { JsonViewer } from "@rich-data/viewer";
import { Typography } from "@mui/material";

export default function Admin() {
  const socket = useSocket();
  const [tabPage, setTabPage] = useState<number>(0);
const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  useEffect(() => {
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 50);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag]);
  const [dataAmount, setDataAmount] = useState<number
  >(0);
  useEffect(() => {
    setDataAmount(
      socket.getByIndex(tabPage).getDataAmount()
    );
  }, [setDataAmount, socket,updateFlag, tabPage]);
  return (
    <>
      <ServerSelectTab
        tabPage={tabPage}
        setTabPage={setTabPage}
        serverNames={socket.raw.map((s) => s.serverName)}
      />
      <Typography variant="body2">
        {dataAmount} Bytes / s
      </Typography>
      <JsonViewer
        value={socket.getByIndex(tabPage)}
        indentWidth={20}
        defaultInspectDepth={1}
      />
    </>
  );
}
