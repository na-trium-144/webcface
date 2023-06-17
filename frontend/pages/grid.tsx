import { Responsive, WidthProvider } from "react-grid-layout";
const ResponsiveGridLayout = WidthProvider(Responsive);
import { useState } from "react";
// 酷い名前だ
import { ShellPageFav, ShellPageServer } from "./shell";
import { LogCard, LogSelectList } from "./log";
import { TerminalLogTable } from "../components/TerminalLog";
import { useSidebarState } from "../components/sidebarContext";
import { useSocket } from "../components/socketContext";
import { CustomPage } from "./custom";
import { useFunctionFavs, FunctionFavs } from "../lib/useFunctionFavs";


import Paper from "@mui/material/Paper";
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
}) => {
  const sx = { width: "100%", height: "100%", p: 1 };
  const sx2 = {};
  if (props.scroll) {
    sx2.overflowX = "hidden";
    sx2.overflowY = "auto";
  } else {
    sx2.overflow = "hidden";
  }
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
        <Grid item xs>
          <Typography
            variant="subtitle2"
            component="span"
            sx={{
              wordBreak: "break-word",
            }}
          >
            {props.title}
          </Typography>
        </Grid>
        <Grid item>
          <IconButton onClick={props.onClose} size="small">
            <CancelIcon fontSize="small" />
          </IconButton>
        </Grid>
      </Grid>
      <div
        style={{
          flexBasis: "auto",
          flexGrow: 1,
          flexShrink: 1,
          width: "100%",
          ...sx2,
        }}
      >
        {props.children}
      </div>
    </Paper>
  );
};

export default function GridMode() {
  const socket = useSocket();

  const [layouts, setLayouts] = useState({});
  const [seriesMulti, setSeriesMulti] = useState<FromRobotSettingServerT[]>([]);
  const { sidebarState, setSidebarState } = useSidebarState();
  const favs = useFunctionFavs();

  return (
    <ResponsiveGridLayout
      className="layout"
      layouts={layouts}
      onLayoutChange={setLayouts}
      breakpoints={{ lg: 1200, md: 996, sm: 768, xs: 480, xxs: 0 }}
      cols={{ lg: 12, md: 10, sm: 6, xs: 4, xxs: 2 }}
    >
      {sidebarState.shellFav && (
        <div key="shellFav" data-grid={{ x: 0, y: 0, w: 10, h: 2 }}>
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

            <ShellPageFav opener={() => {}} favs={favs}/>
          </GridPaper>
        </div>
      )}
      {socket.raw.map(
        (s, si) =>
          sidebarState.shell[si] && (
            <div key={`shell${si}`} data-grid={{ x: 0, y: 0, w: 10, h: 3 }}>
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
                <ShellPageServer sid={si} opener={() => {}} favs={favs}/>
              </GridPaper>
            </div>
          )
      )}
      {socket.raw.map(
        (s, si) =>
          sidebarState.terminalLog[si] && (
            <div
              key={`terminallog${si}`}
              data-grid={{ x: 0, y: 0, w: 10, h: 4 }}
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
        <div key="graphselect" data-grid={{ x: 0, y: 0, w: 2, h: 3 }}>
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
              data-grid={{ x: 0, y: 0, w: 2, h: 2 }}
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
                  data-grid={{ x: 0, y: 0, w: 5, h: 3 }}
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
    </ResponsiveGridLayout>
  );
}
