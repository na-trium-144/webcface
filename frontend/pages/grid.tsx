import GridLayout from "react-grid-layout";
import { useContext, useState, useEffect, useRef } from "react";
// 酷い名前だ
import { ShellPageFav, ShellPageServer } from "./shell";
import { LogCard, LogSelectList } from "./log";
import { TerminalLogTable } from "../components/TerminalLog";
import { useSidebarState } from "../components/sidebarContext";
import { useSocket } from "../components/socketContext";
import { CustomPage } from "./custom";
import { CameraImage } from "./camera";
import { useFunctionFavs, FunctionFavs } from "../lib/useFunctionFavs";
import { GamepadsContext } from "react-gamepads";
import { GamepadView } from "../components/gamepadView";

import Paper from "@mui/material/Paper";
import Divider from "@mui/material/Divider";
import Box from "@mui/material/Box";
import BookmarkIcon from "@mui/icons-material/Bookmark";
import Typography from "@mui/material/Typography";
import CancelIcon from "@mui/icons-material/Cancel";
import IconButton from "@mui/material/IconButton";
import Grid from "@mui/material/Grid";

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
  onClick: () => void;
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
      onClick={props.onClick}
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
  const [lsLayouts, setLsLayouts] = useState<any[]>([]);
  const [lsInit, setLsInit] = useState<boolean>(false);
  useEffect(() => {
    const i = setInterval(() => {
      if (lsInit) {
        let lsLayoutsCurrent = lsLayouts;
        let lsChanged = false;
        for (const ln of layouts.current) {
          const li = prevLayouts.current.findIndex((l) => l.i === ln.i);
          const lsi = lsLayouts.findIndex((ls) => ls.i === ln.i);
          if (li === -1 && lsi >= 0) {
            // 新規ウィンドウ→newLayoutsを直接書き換えてlsから復元
            ln.x = lsLayouts[lsi].x;
            ln.y = lsLayouts[lsi].y;
            ln.w = lsLayouts[lsi].w;
            ln.h = lsLayouts[lsi].h;
          }
          if (
            li === -1 ||
            prevLayouts.current[li].x !== ln.x ||
            prevLayouts.current[li].y !== ln.y ||
            prevLayouts.current[li].w !== ln.w ||
            prevLayouts.current[li].h !== ln.h
          ) {
            //手動で変更されたときのみlsを更新する
            if (lsi >= 0) {
              lsLayoutsCurrent[lsi] = ln;
            } else {
              lsLayoutsCurrent.push(ln);
            }
            lsChanged = true;
          }
          if (li === -1) {
            //新しいウィンドウを一番前にする
            updateZ(ln.i)();
          }
        }
        for (const l of prevLayouts.current) {
          const lni = layouts.current.findIndex((ln) => ln.i === l.i);
          const lsi = lsLayoutsCurrent.findIndex((ls) => ls.i === l.i);
          if (lni === -1) {
            // 消えたウィンドウはlsから消す
            lsLayoutsCurrent = lsLayoutsCurrent
              .slice(0, lsi)
              .concat(lsLayoutsCurrent.slice(lsi + 1));
            lsChanged = true;
          }
        }
        if (lsChanged) {
          saveLsLayouts(lsLayoutsCurrent.map((l) => ({ ...l })));
        }
        prevLayouts.current = layouts.current.map((l) => ({ ...l }));
        if (width !== parentDiv.current.clientWidth) {
          const newWidth = parentDiv.current.clientWidth;
          const newCols = Math.ceil(newWidth / 120);
          setWidth(newWidth);
          setCols(newCols);
          // layoutを直接書き換える(理由はonLayoutChangeを参照)

          for (const ln of layouts.current) {
            const li = prevLayouts.current.findIndex((l) => l.i === ln.i);
            const lsi = lsLayouts.findIndex((ls) => ls.i === ln.i);
            // lsのサイズを復元 (手動でいじらない限りlsは変わらないので)
            if (lsi >= 0) {
              ln.x = lsLayouts[lsi].x;
              ln.y = lsLayouts[lsi].y;
              ln.w = lsLayouts[lsi].w;
              ln.h = lsLayouts[lsi].h;
            }
            if (ln.x + ln.w > newCols) {
              if (ln.w > newCols) {
                ln.x = 0;
                ln.w = newCols;
              } else {
                ln.x = newCols - ln.w;
              }
            }
            prevLayouts.current[li] = { ...ln };
          }
        }
      }
    }, 100);
    return () => clearInterval(i);
  }, [width, parentDiv, setWidth, lsLayouts, lsInit]);

  const [seriesMulti, setSeriesMulti] = useState<FromRobotSettingServerT[]>([]);
  const { sidebarState, setSidebarState } = useSidebarState();
  const favs = useFunctionFavs();

  const lsKey = "RC23WebCon-Grid";
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
  };

  const dataGrid = (key: string) => {
    const l = lsLayouts.find((ls) => ls.i === key);
    if (l) {
      return { x: l.x, y: l.y, w: l.w, h: l.h };
    } else {
      return { x: 0, y: 0 }; //wとhのデフォルト値は別で設定
    }
  };

  const [z, setZ] = useState<object>({});
  const maxZ = useRef<number>(0);
  const updateZ = (name: string) => () =>
    setZ((z) => {
      z[name] = { zIndex: ++maxZ.current };
      return { ...z };
    });
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
            style={z["shellFav"]}
          >
            <GridPaper
              onClick={updateZ(`shellFav`)}
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
                key={`shell-${s.serverName}`}
                data-grid={{
                  w: cols,
                  h: 3,
                  ...dataGrid(`shell-${s.serverName}`),
                }}
                style={z[`shell-${s.serverName}`]}
              >
                <GridPaper
                  onClick={updateZ(`shell-${s.serverName}`)}
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
                key={`terminallog-${s.serverName}`}
                data-grid={{
                  w: cols,
                  h: 3,
                  ...dataGrid(`terminallog-${s.serverName}`),
                }}
                style={z[`terminallog-${s.serverName}`]}
              >
                <GridPaper
                  onClick={updateZ(`terminallog-${s.serverName}`)}
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
            style={z[`graphselect`]}
          >
            <GridPaper
              onClick={updateZ(`graphselect`)}
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
                key={`graph-${s.serverName}:${el.name}`}
                data-grid={{
                  w: 2,
                  h: 3,
                  ...dataGrid(`graph-${s.serverName}:${el.name}`),
                }}
                style={z[`graph-${s.serverName}:${el.name}`]}
              >
                <GridPaper
                  onClick={updateZ(`graph-${s.serverName}:${el.name}`)}
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
            prev.concat(
              s.getCustomPageLayout().map(
                (l, li) =>
                  sidebarState.custom[`${si}:${li}`] && (
                    <div
                      key={`custom-${s.serverName}:${l.name}`}
                      data-grid={{
                        w: 4,
                        h: 5,
                        ...dataGrid(`custom-${s.serverName}:${l.name}`),
                      }}
                      style={z[`custom-${s.serverName}:${l.name}`]}
                    >
                      <GridPaper
                        onClick={updateZ(`custom-${s.serverName}:${l.name}`)}
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
                  )
              )
            ),
          []
        )}
        {socket.raw.reduce(
          (prev, s, si) =>
            prev.concat(
              s.getImageData().map(
                (im, ii) =>
                  sidebarState.image[`${si}:${ii}`] && (
                    <div
                      key={`image-${s.serverName}:${im.name}`}
                      data-grid={{
                        w: 4,
                        h: 5,
                        ...dataGrid(`image-${s.serverName}:${im.name}`),
                      }}
                      style={z[`image-${s.serverName}:${im.name}`]}
                    >
                      <GridPaper
                        onClick={updateZ(`image-${s.serverName}:${im.name}`)}
                        scroll={true}
                        title={`${s.serverName}:${im.name}`}
                        onClose={() =>
                          setSidebarState((sidebarState) => {
                            sidebarState.image[`${si}:${ii}`] = false;
                            return { ...sidebarState };
                          })
                        }
                      >
                        <CameraImage sid={si} iid={ii} />
                      </GridPaper>
                    </div>
                  )
              )
            ),
          []
        )}
        {Object.entries(gamepads).reduce(
          (prev, [gi, g]) =>
            prev.concat(
              sidebarState.gamepad[gi] && (
                <div
                  key={`gamepad-${g.id}`}
                  data-grid={{
                    w: Math.min(6, cols),
                    h: 3,
                    ...dataGrid(`gamepad-${g.id}`),
                  }}
                  style={z[`gamepad-${g.id}`]}
                >
                  <GridPaper
                    onClick={updateZ(`gamepad-${g.id}`)}
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
            ),
          []
        )}
      </GridLayout>
    </div>
  );
}
