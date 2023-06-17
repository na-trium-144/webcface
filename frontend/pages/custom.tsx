import { Grid } from "@mui/material";
// import Field from "../components/field";
// import ControlPanel from "../components/ControlPanel";
import { useRouter } from "next/router";
import { useSocket } from "../components/socketContext";
import { CustomPageLayoutSettingT } from "../lib/global";
import { useState, useEffect } from "react";

// https://github.com/konvajs/react-konva/issues/588
import dynamic from "next/dynamic";
const ControlPanel = dynamic(() => import("../components/ControlPanel"), {
  ssr: false,
});

export const CustomPage = (props: { id: number; sid: number }) => {
  const { id, sid } = props;
  const socket = useSocket();
  const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  useEffect(() => {
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 50);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag]);
  const [customPageLayout, setCustomPageLayout] = useState<
    CustomPageLayoutSettingT | undefined
  >(undefined);
  useEffect(() => {
    setCustomPageLayout(socket.getByIndex(sid).getCustomPageLayout()[id]);
  }, [setCustomPageLayout, socket, id, sid, updateFlag, updateFlag]);
  if (customPageLayout == undefined) {
    return <></>;
  } else {
    return (
      <ControlPanel
        layout={socket.getByIndex(sid).getCustomPageLayout()[id].layout}
        sid={sid}
      />
    );
  }
};
export default function TournamentPage() {
  const router = useRouter();
  const sid = router.query.sid == undefined ? 0 : parseInt(router.query.sid);
  const id = router.query.id == undefined ? 0 : parseInt(router.query.id);
  return <CustomPage id={id} sid={sid} />;
}
