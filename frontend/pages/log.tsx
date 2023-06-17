import React, { useEffect, useState, useRef } from "react";
import {
  Card,
  CardContent,
  Typography,
  Grid,
  List,
  ListItem,
  ListItemButton,
  ListItemText,
  ListItemIcon,
  Checkbox,
  Box,
} from "@mui/material";
import FolderTwoToneIcon from "@mui/icons-material/FolderTwoTone";
import FolderOpenTwoToneIcon from "@mui/icons-material/FolderOpenTwoTone";
import RouterTwoToneIcon from "@mui/icons-material/RouterTwoTone";
import RouterOutlinedIcon from "@mui/icons-material/RouterOutlined";
import { SingleLine } from "../components/logPage/element/graph";
import TextLog from "../components/logPage/element/textLog";
import { useSocket } from "../components/socketContext";
import {
  FromRobotSettingT,
  FromRobotDataT,
  FromRobotSettingGroupT,
  FromRobotSettingServerT,
} from "../lib/global";

const hideZoomOption = {
  chart: {
    zoom: {
      enabled: false,
    },
  },
};

export const LogCard = (props: {
  serverIndex: number;
  sensor: FromRobotSettingT;
  hideTitle?: boolean;
}) => {
  // console.log("LogCard construction");
  const { serverIndex, sensor, hideTitle } = props;
  const [dateStr, setDateStr] = useState<string>("");
  const [fromRobotDataOne, setFromRobotDataOne] = useState<FromRobotDataT>({
    name: "",
    data: [], //もともとdataは最新の1つだけを入れるはずだったのを配列に変更したが名前にはOneと書かれたまま
    prevdata: null,
    timestamp: new Date(),
  });
  const socket = useSocket();
  const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  useEffect(() => {
    const fromRobotData = socket.getByIndex(serverIndex).getFromRobotData();
    const idx = fromRobotData.findIndex((el) => el.name === sensor.name);
    if (idx !== -1) {
      setFromRobotDataOne(fromRobotData[idx]);
      const lastDate: Date = fromRobotData[idx].timestamp;
      setDateStr(lastDate.toISOString().substr(11, 12));
    }
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 30);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag, socket, sensor, serverIndex]);

  return (
    <div
      style={{
        width: "100%",
        height: "100%",
        display: "flex",
        flexDirection: "column",
      }}
    >
      {hideTitle || (
        <Typography
          variant="h6"
          align="center"
          sx={{
            wordBreak: "break-word",
            flexBasis: "auto",
            flexGrow: 0,
            flexShrink: 0,
          }}
        >
          {`${socket.getByIndex(serverIndex).serverName}:${sensor.name}`}
        </Typography>
      )}
      {sensor.type !== "int" && sensor.type !== "float" ? (
        <TextLog
          title={`${socket.getByIndex(serverIndex).serverName}:${sensor.name}`}
          data={fromRobotDataOne.lastdata}
        />
      ) : (
        <SingleLine
          title={`${socket.getByIndex(serverIndex).serverName}:${sensor.name}`}
          series={{
            name: fromRobotDataOne.name,
            data: fromRobotDataOne.data,
            timestamp: fromRobotDataOne.timestamp,
          }}
          options={hideZoomOption}
        />
      )}
      <Typography
        variant="body2"
        sx={{ flexBasis: "auto", flexGrow: 0, flexShrink: 0 }}
      >
        {" "}
        value=
        {fromRobotDataOne.lastdata} @ {dateStr}{" "}
      </Typography>
    </div>
  );
};

const fromRobotSettingParse = (fromRobotSetting: FromRobotSettingT[]) => {
  let prevNameSplit: string[] = [];
  const ret: (FromRobotSettingT | FromRobotSettingGroupT)[] = [];
  for (let i = 0; i < fromRobotSetting.length; i++) {
    const newNameSplit: string[] = fromRobotSetting[i].name.split(".");
    for (let d = 0; d < newNameSplit.length - 1; d++) {
      if (prevNameSplit.length <= d || prevNameSplit[d] !== newNameSplit[d]) {
        ret.push({ name: newNameSplit.slice(0, d + 1).join(".") });
      }
    }
    ret.push(fromRobotSetting[i]);
    prevNameSplit = newNameSplit;
  }
  return ret;
};
const filterOpenedElements = (
  series: (FromRobotSettingT | FromRobotSettingGroupT)[]
) => {
  const ret: number[] = []; // openな要素のindexのリストを返す
  let prevNameSplit: string[] = [];
  let prevOpened: boolean[] = [];
  for (let i = 0; i < series.length; i++) {
    const newNameSplit: string[] = series[i].name.split(".");
    for (let d = 0; d < newNameSplit.length; d++) {
      if (prevNameSplit.length <= d || prevNameSplit[d] !== newNameSplit[d]) {
        prevOpened = prevOpened.slice(0, d);
        break;
      }
    }
    if (prevOpened.indexOf(false) < 0) {
      ret.push(i);
    }
    for (let d = 0; d < newNameSplit.length; d++) {
      if (prevNameSplit.length <= d || prevNameSplit[d] !== newNameSplit[d]) {
        if (series[i].type == undefined) {
          //group
          prevOpened = prevOpened.slice(0, d).concat([!!series[i].isOpened]);
        }
      }
    }
    prevNameSplit = newNameSplit;
  }
  return ret;
};

export const LogSelectList = (props: {
  seriesMulti: FromRobotSettingServerT[];
  setSeriesMulti: (
    callback: (
      seriesMulti: FromRobotSettingServerT[]
    ) => FromRobotSettingServerT[]
  ) => void;
}) => {
  const { seriesMulti, setSeriesMulti } = props;
  const socket = useSocket();

  useEffect(() => {
    const slicedMulti: FromRobotSettingServerT[] = [];
    for (const s of socket.raw) {
      // const exceptions: string[] = ["lrf", "robot_pos", "robot_vel", "image"];
      const sliced: (FromRobotSettingT | FromRobotSettingGroupT)[] =
        fromRobotSettingParse(
          s.fromRobotSetting
          // .filter((el) => !exceptions.includes(el.name))
        );
      slicedMulti.push({
        name: s.serverName,
        elements: sliced,
        isOpened: false,
      });
    }
    setSeriesMulti(() => slicedMulti);
  }, [socket]);

  return (
    <List>
      <ListItem dense>
        <ListItemText>変数名</ListItemText>
      </ListItem>
      {seriesMulti.map((ss, si) => (
        <>
          <ListItemButton
            onClick={() =>
              setSeriesMulti((seriesMulti) => {
                seriesMulti[si].isOpened = !seriesMulti[si].isOpened;
                return seriesMulti.slice();
              })
            }
            dense
            key={si}
          >
            <ListItemIcon sx={{ minWidth: "40px" }}>
              <Checkbox
                icon={<RouterOutlinedIcon />}
                checkedIcon={<RouterTwoToneIcon />}
                edge="start"
                checked={ss.isOpened}
                sx={{ paddingTop: 0, paddingBottom: 0 }}
                disableRipple
              />
            </ListItemIcon>
            <ListItemText primary={ss.name} />
          </ListItemButton>
          {ss.isOpened &&
            filterOpenedElements(ss.elements).map((i) => (
              <ListItem key={`${si}:${i}`} disablePadding>
                <Box
                  sx={{
                    paddingLeft:
                      2 * (ss.elements[i].name.split(".").length + 1),
                  }}
                >
                  {ss.elements[i].type != undefined ? (
                    <ListItemButton
                      onClick={() =>
                        setSeriesMulti((seriesMulti) => {
                          seriesMulti[si].elements[i].isSelected =
                            !seriesMulti[si].elements[i].isSelected;
                          return seriesMulti.slice();
                        })
                      }
                      dense
                    >
                      <ListItemIcon sx={{ minWidth: "40px" }}>
                        <Checkbox
                          edge="start"
                          checked={!!ss.elements[i].isSelected}
                          tabIndex={-1}
                          sx={{ paddingTop: 0, paddingBottom: 0 }}
                          disableRipple
                        />
                      </ListItemIcon>
                      <ListItemText
                        primary={ss.elements[i].name.split(".").pop()}
                      />
                    </ListItemButton>
                  ) : (
                    <ListItemButton
                      onClick={() =>
                        setSeriesMulti((seriesMulti) => {
                          seriesMulti[si].elements[i].isOpened =
                            !seriesMulti[si].elements[i].isOpened;
                          return seriesMulti.slice();
                        })
                      }
                      dense
                    >
                      <ListItemIcon sx={{ minWidth: "40px" }}>
                        <Checkbox
                          icon={<FolderTwoToneIcon />}
                          checkedIcon={<FolderOpenTwoToneIcon />}
                          edge="start"
                          checked={!!ss.elements[i].isOpened}
                          tabIndex={-1}
                          sx={{ paddingTop: 0, paddingBottom: 0 }}
                          disableRipple
                        />
                      </ListItemIcon>
                      <ListItemText
                        primary={ss.elements[i].name.split(".").pop()}
                      />
                    </ListItemButton>
                  )}
                </Box>
              </ListItem>
            ))}
        </>
      ))}
    </List>
  );
};
export default function Log() {
  // 変数の一覧
  // 2次元配列、1次元目がサーバーごと、2次元目が変数ごと
  // フォルダと変数本体をそれぞれ1要素で表す
  /*
  [
    [ // for サーバー1
      {var1}, // フォルダ
      {var1.value1}, //変数
      {var1.value2}, //変数
    ],
    [ // for サーバー2
      ...
    ],
  ]
  */
  const [seriesMulti, setSeriesMulti] = useState<FromRobotSettingServerT[]>([]);
  return (
    <Grid container spacing={1}>
      <Grid item xs={6} md={4} lg={3}>
        <LogSelectList
          seriesMulti={seriesMulti}
          setSeriesMulti={setSeriesMulti}
        />
      </Grid>
      <Grid item xs>
        <Grid container spacing={1}>
          {seriesMulti.map((ss, si) =>
            ss.elements
              .filter((el) => el.isSelected)
              .map((el) => (
                <Grid
                  item
                  xs={12}
                  md={6}
                  lg={4}
                  xl={3}
                  key={`${si}:${el.name}`}
                >
                  <LogCard serverIndex={si} sensor={el}></LogCard>
                </Grid>
              ))
          )}
        </Grid>
      </Grid>
    </Grid>
  );
}
