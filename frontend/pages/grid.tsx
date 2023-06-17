import GridLayout from "react-grid-layout";
import { useContext, useState, useEffect, useRef } from "react";
// 酷い名前だ
import { ShellPageFav, ShellPageServer } from "./shell";
import { LogCard, LogSelectList } from "./log";
import { TerminalLogTable } from "../components/TerminalLog";
import { useSidebarState } from "../components/sidebarContext";
import { useSocket } from "../components/socketContext";
import { CustomPage } from "./custom";
import { useFunctionFavs, FunctionFavs } from "../lib/useFunctionFavs";
import { GamepadsContext } from "react-gamepads";

import Paper from "@mui/material/Paper";
import Divider from "@mui/material/Divider";
import Box from "@mui/material/Box";
import BookmarkIcon from "@mui/icons-material/Bookmark";
import Typography from "@mui/material/Typography";
import CancelIcon from "@mui/icons-material/Cancel";
import IconButton from "@mui/material/IconButton";
import Grid from "@mui/material/Grid";
import InputLabel from "@mui/material/InputLabel";
import MenuItem from "@mui/material/MenuItem";
import FormControl from "@mui/material/FormControl";
import Button from "@mui/material/Button";
import ButtonGroup from "@mui/material/ButtonGroup";
import Select, { SelectChangeEvent } from "@mui/material/Select";
import Slider from "@mui/material/Slider";

import {
  FromRobotSettingT,
  FromRobotDataT,
  FromRobotSettingGroupT,
  FromRobotSettingServerT,
} from "../lib/global";

const GridPaper = (props: {
  scroll?: boolean;
  title: string;
  onClose: () => void;
  children: any;
}) => {
  const sx = { width: "100%", height: "100%" };
  const sx2 = {};
  if (props.scroll) {
    sx2.overflowX = "hidden";
    sx2.overflowY = "auto";
  } else {
    sx2.overflow = "hidden";
  }
  const [grabbing, setGrabbing] = useState<boolean>(false);
  return (
    <Paper
      elevation={3}
      sx={{ ...sx, display: "flex", flexDirection: "column" }}
    >
      <Grid
        container
        alignItems="center"
        sx={{ flexBasis: "auto", flexGrow: 0, flexShrink: 0 }}
      >
        <Grid item xs sx={{ p: 1, pb: 0 }}>
          <div
            className="drag-handle"
            style={{ cursor: grabbing ? "grabbing" : "grab" }}
            onMouseDown={() => setGrabbing(true)}
            onMouseUp={() => setGrabbing(false)}
          >
            <Typography
              variant="subtitle2"
              component="span"
              sx={{
                wordBreak: "break-word",
              }}
            >
              {props.title}
            </Typography>
          </div>
        </Grid>
        <Grid item sx={{ p: 1, pb: 0 }}>
          <IconButton onClick={props.onClose} size="small">
            <CancelIcon fontSize="small" />
          </IconButton>
        </Grid>
        <Grid item xs={12}>
          <Divider />
        </Grid>
      </Grid>
      <Box
        sx={{
          flexBasis: "auto",
          flexGrow: 1,
          flexShrink: 1,
          width: "100%",
          p: 1,
          ...sx2,
        }}
      >
        {props.children}
      </Box>
    </Paper>
  );
};

export default function GridMode() {
  const socket = useSocket();
  const { gamepads } = useContext(GamepadsContext);

  // const [layouts, setLayouts] = useState<any[]>([]);
  const layouts = useRef<any[]>([]);
  const prevLayouts = useRef<any[]>([]);
  const [width, setWidth] = useState<number>(0);
  const [cols, setCols] = useState<number>(1);
  const parentDiv = useRef();
  useEffect(() => {
    const i = setInterval(() => {
      if (width !== parentDiv.current.clientWidth) {
        const newWidth = parentDiv.current.clientWidth;
        const newCols = Math.ceil(newWidth / 120);
        setWidth(newWidth);
        setCols(newCols);
        // layoutを直接書き換える(理由はonLayoutChangeを参照)
        for (const l of layouts.current) {
          if (l.w > newCols) {
            l.x = 0;
            l.w = newCols;
          } else if (l.x + l.w > newCols) {
            l.x = newCols - l.w;
          }
        }
      }
    }, 100);
    return () => clearInterval(i);
  }, [width, parentDiv, setWidth]);

  const [seriesMulti, setSeriesMulti] = useState<FromRobotSettingServerT[]>([]);
  const { sidebarState, setSidebarState } = useSidebarState();
  const favs = useFunctionFavs();

  const lsKey = "RC23WebCon-Grid";
  const [lsLayouts, setLsLayouts] = useState<any[]>([]);
  const [lsInit, setLsInit] = useState<boolean>(false);
  const loadLsLayouts = () => {
    try {
      return JSON.parse(window.localStorage.getItem(lsKey)) || [];
    } catch {
      return [];
    }
  };
  useEffect(() => {
    setLsLayouts(loadLsLayouts());
    setLsInit(true);
  }, []);
  const saveLsLayouts = (layouts: object) => {
    try {
      window.localStorage.setItem(lsKey, JSON.stringify(layouts));
    } catch {}
    setLsLayouts(layouts);
  };
  const [layoutChanged, setLayoutChanged] = useState<boolean>(false);
  const onLayoutChange = (newLayouts: any[]) => {
    // newLayoutsにはGridLayoutから渡されるLayoutオブジェクトが入っているが、
    // これの中身を直接書き換えるとなぜかレイアウトが変わる
    // しかしこれをsliceやmapしてから(別のオブジェクトとして)
    // GridLayoutのlayoutに渡すと反映されない! なんだこの仕様
    // (layoutsプロパティは初期配置のためだけにあるってこと?)
    // 関連しそうなissue
    //  https://github.com/react-grid-layout/react-grid-layout/issues/1563
    //  https://github.com/react-grid-layout/react-grid-layout/issues/1583
    //  https://github.com/react-grid-layout/react-grid-layout/issues/1625

    layouts.current = newLayouts; // newLayoutsの参照を渡す
    setLayoutChanged(true);
  };
  useEffect(() => {
    if (layoutChanged && lsInit) {
      for (const ln of layouts.current) {
        const li = prevLayouts.current.findIndex((l) => l.i === ln.i);
        const lsi = lsLayouts.findIndex((ls) => ls.i === ln.i);
        if (li === -1 && lsi >= 0) {
          // newLayoutsを直接書き換える
          ln.x = lsLayouts[lsi].x;
          ln.y = lsLayouts[lsi].y;
          ln.w = lsLayouts[lsi].w;
          ln.h = lsLayouts[lsi].h;
        }
        if (lsi >= 0) {
          lsLayouts[lsi] = ln;
        } else {
          lsLayouts.push(ln);
        }
      }
      let lsLayoutsCurrent = lsLayouts;
      for (const l of prevLayouts.current) {
        const lni = layouts.current.findIndex((ln) => ln.i === l.i);
        const lsi = lsLayoutsCurrent.findIndex((ls) => ls.i === l.i);
        if (lni === -1) {
          lsLayoutsCurrent = lsLayoutsCurrent
            .slice(0, lsi)
            .concat(lsLayoutsCurrent.slice(lsi + 1));
        }
      }
      saveLsLayouts(lsLayoutsCurrent.map((l) => ({ ...l })));
      prevLayouts.current = layouts.current.map((l) => ({ ...l }));
      setLayoutChanged(false);
    }
  }, [lsLayouts, lsInit, layoutChanged]);

  const dataGrid = (key: string) => {
    const l = lsLayouts.find((ls) => ls.i === key);
    if (l) {
      return { x: l.x, y: l.y, w: l.w, h: l.h };
    } else {
      return { x: 0, y: 0 }; //wとhのデフォルト値は別で設定
    }
  };
  return (
    <div style={{ height: "100%", overflow: "hidden" }} ref={parentDiv}>
      <GridLayout
        className="layout"
        onLayoutChange={onLayoutChange}
        cols={cols}
        rowHeight={80}
        width={width}
        draggableHandle=".drag-handle"
        compactType="null"
        allowOverlap
        preventCollision
      >
        {sidebarState.shellFav && (
          <div
            key="shellFav"
            data-grid={{ w: cols, h: 3, ...dataGrid("shellFav") }}
          >
            <GridPaper
              scroll={true}
              title="シェル関数(登録済み)"
              onClose={() =>
                setSidebarState((sidebarState) => ({
                  ...sidebarState,
                  shellFav: false,
                }))
              }
            >
              <Box>
                <BookmarkIcon sx={{ fontSize: 18 }} color="primary" />
                <Typography variant="caption" gutterBottom>
                  を押したものが表示されます
                </Typography>
              </Box>

              <ShellPageFav opener={() => {}} favs={favs} />
            </GridPaper>
          </div>
        )}
        {socket.raw.map(
          (s, si) =>
            sidebarState.shell[si] && (
              <div
                key={`shell${si}`}
                data-grid={{ w: cols, h: 3, ...dataGrid(`shell${si}`) }}
              >
                <GridPaper
                  scroll={true}
                  title={`${s.serverName}:シェル関数`}
                  onClose={() =>
                    setSidebarState((sidebarState) => {
                      sidebarState.shell[si] = false;
                      return { ...sidebarState };
                    })
                  }
                >
                  <ShellPageServer sid={si} opener={() => {}} favs={favs} />
                </GridPaper>
              </div>
            )
        )}
        {socket.raw.map(
          (s, si) =>
            sidebarState.terminalLog[si] && (
              <div
                key={`terminallog${si}`}
                data-grid={{ w: cols, h: 3, ...dataGrid(`terminallog${si}`) }}
              >
                <GridPaper
                  scroll={true}
                  title={`${s.serverName}:ログ出力`}
                  onClose={() =>
                    setSidebarState((sidebarState) => {
                      sidebarState.terminalLog[si] = false;
                      return { ...sidebarState };
                    })
                  }
                >
                  <TerminalLogTable sid={si} />
                </GridPaper>
              </div>
            )
        )}
        {sidebarState.logSelect && (
          <div
            key="graphselect"
            data-grid={{ w: 2, h: 5, ...dataGrid("graphselect") }}
          >
            <GridPaper
              scroll={true}
              title="グラフ表示"
              onClose={() =>
                setSidebarState((sidebarState) => ({
                  ...sidebarState,
                  logSelect: false,
                }))
              }
            >
              <LogSelectList
                seriesMulti={seriesMulti}
                setSeriesMulti={setSeriesMulti}
              />
            </GridPaper>
          </div>
        )}
        {seriesMulti.map((ss, si) =>
          ss.elements
            .filter((el) => el.isSelected)
            .map((el) => (
              <div
                key={`graph${si}:${el.name}`}
                data-grid={{ w: 2, h: 3, ...dataGrid(`graph${si}:${el.name}`) }}
              >
                <GridPaper
                  title={`${ss.name}:${el.name}`}
                  onClose={() =>
                    setSeriesMulti((seriesMulti) => {
                      el.isSelected = false;
                      return seriesMulti.slice();
                    })
                  }
                >
                  <LogCard serverIndex={si} sensor={el} hideTitle={true} />
                </GridPaper>
              </div>
            ))
        )}
        {socket.raw.reduce(
          (prev, s, si) =>
            s.getCustomPageLayout().map((l, li) =>
              prev.concat([
                sidebarState.custom[`${si}:${li}`] && (
                  <div
                    key={`custom${si}:${li}`}
                    data-grid={{
                      w: 4,
                      h: 5,
                      ...dataGrid(`custom${si}:${li}`),
                    }}
                  >
                    <GridPaper
                      scroll={true}
                      title={`${s.serverName}:${l.name}`}
                      onClose={() =>
                        setSidebarState((sidebarState) => {
                          sidebarState.custom[`${si}:${li}`] = false;
                          return { ...sidebarState };
                        })
                      }
                    >
                      <CustomPage sid={si} id={li} />
                    </GridPaper>
                  </div>
                ),
              ])
            ),
          []
        )}
        {Object.entries(gamepads).map(
          ([gi, g]) =>
            sidebarState.gamepad[gi] && (
              <div
                key={gi}
                data-grid={{ w: Math.min(6, cols), h: 3, ...dataGrid(gi) }}
              >
                <GridPaper
                  scroll={true}
                  title={g.id}
                  onClose={() =>
                    setSidebarState((sidebarState) => {
                      sidebarState.gamepad[gi] = false;
                      return { ...sidebarState };
                    })
                  }
                >
                  <GamepadView gi={gi} />
                </GridPaper>
              </div>
            )
        )}
      </GridLayout>
    </div>
  );
}

const GamepadView = (props: { gi: number }) => {
  const socket = useSocket();
  const { gamepads } = useContext(GamepadsContext);
  const { gi } = props;
  const connectedServer = socket.raw.findIndex(
    (s) => s.gamepadConnectIndex === gi
  );
  const setConnectedServer = (si: number | null) => {
    if (connectedServer != null && connectedServer >= 0) {
      socket.getByIndex(connectedServer).setGamepadConnectIndex(null);
    }
    if (si >= 0) {
      socket.getByIndex(si).setGamepadConnectIndex(gi);
    }
  };
  return (
    <>
      <Grid container alignItems="center">
        <Grid item>バックエンドに接続:</Grid>
        <Grid item xs>
          <FormControl fullWidth>
            <InputLabel id={`GamepadViweSelect${gi}`}>接続先</InputLabel>
            <Select
              labelId={`GamepadViweSelect${gi}`}
              value={connectedServer}
              label="接続先"
              onChange={(e) => {
                setConnectedServer(e.target.value);
              }}
              size="small"
            >
              <MenuItem value={-1}>なし</MenuItem>
              {socket.raw.map((s, si) => (
                <MenuItem value={si} key={si}>
                  {s.serverName}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
      </Grid>
      <div>
        buttons
        <ButtonGroup variant="contained" size="small">
          {gamepads[gi].buttons.map((b, bi) => (
            <Button disabled={!b.pressed} key={bi}>
              {bi}
            </Button>
          ))}
        </ButtonGroup>
      </div>
      <div>
        axes
        {gamepads[gi].axes.map((b, bi) => (
          <Grid container>
            <Grid item>{bi}:</Grid>
            <Grid item xs>
              <Slider
                size="small"
                value={b * 50 + 50}
                valueLabelDisplay="none"
              />
            </Grid>
          </Grid>
        ))}
      </div>
    </>
  );
};
