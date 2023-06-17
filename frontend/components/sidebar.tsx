import SportsScoreIcon from "@mui/icons-material/SportsScore";
import SportsEsportsIcon from "@mui/icons-material/SportsEsports";
import KeyboardIcon from "@mui/icons-material/Keyboard";
import AutoGraphIcon from "@mui/icons-material/AutoGraph";
import PhotoCameraIcon from "@mui/icons-material/PhotoCamera";
import SettingsIcon from "@mui/icons-material/Settings";
import VideogameAssetIcon from "@mui/icons-material/VideogameAsset";
import TerminalIcon from "@mui/icons-material/Terminal";
import ExpandLess from "@mui/icons-material/ExpandLess";
import ExpandMore from "@mui/icons-material/ExpandMore";
import Collapse from "@mui/material/Collapse";
import BookmarkIcon from "@mui/icons-material/Bookmark";
import RouterOutLinedIcon from "@mui/icons-material/RouterOutlined";
import DashboardOutlinedIcon from "@mui/icons-material/DashboardOutlined";
import WysiwygIcon from '@mui/icons-material/Wysiwyg';

import NextLink from "next/link";
import {
  Box,
  ListItemButton,
  ListItemIcon,
  ListItem,
  ListItemText,
  Divider,
} from "@mui/material";
import { useRouter } from "next/router";
import { useSocket } from "../components/socketContext";
import { useEffect, useState } from "react";
import { useSidebarState } from "../components/sidebarContext";

type Props = {
  visible: boolean;
};

const SidebarButton = (props: {
  selected?: boolean;
  open?: boolean;
  showOpenIcon?: boolean;
  onClick: () => void;
  name: string;
  icon: any;
  visible: boolean;
  pl?: number;
}) => {
  return (
    <ListItemButton
      dense
      selected={!!props.selected}
      onClick={props.onClick}
      sx={{ pl: props.pl + 2 || 2 }}
    >
      {props.icon != undefined && (
        <ListItemIcon sx={{ minWidth: "40px" }}>{props.icon}</ListItemIcon>
      )}
      {props.visible && <ListItemText primary={props.name} />}
      {props.visible &&
        props.showOpenIcon &&
        (props.open ? <ExpandLess /> : <ExpandMore />)}
    </ListItemButton>
  );
};
const SidebarButtonCollapse = (props: {
  name: string;
  icon: any;
  visible: boolean;
  contents: {
    name: string;
    icon: any;
    selected: boolean;
    onClick: () => void;
  }[];
}) => {
  const [open, setOpen] = useState<boolean>(false);
  return (
    <>
      <SidebarButton
        onClick={() => setOpen(!open)}
        name={props.name}
        icon={props.icon}
        visible={props.visible}
        open={open}
        showOpenIcon={true}
      />
      <Collapse in={open && props.visible}>
        {props.contents.map((c, si) => (
          <SidebarButton
            selected={c.selected}
            onClick={c.onClick}
            name={c.name}
            icon={c.icon}
            visible={props.visible}
            pl={2}
            key={si}
          />
        ))}
      </Collapse>
    </>
  );
};

// export function Sidebar(props) {
export default function Sidebar(props: Props) {
  const { pathname } = useRouter();
  const { sidebarState, setSidebarState } = useSidebarState();
  const socket = useSocket();
  useEffect(() => {
    if (sidebarState.shell.length < socket.raw.length) {
      setSidebarState((sidebarState) => {
        while (sidebarState.shell.length < socket.raw.length) {
          sidebarState.shell.push(false);
        }
        while (sidebarState.terminalLog.length < socket.raw.length) {
          sidebarState.terminalLog.push(false);
        }
        return { ...sidebarState };
      });
    }
  }, [socket, sidebarState, setSidebarState]);

  const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  const [layouts, setLayouts] = useState<
    {
      name: string;
      sid: number;
      id: number;
    }[]
  >([]);
  useEffect(() => {
    const i = setTimeout(() => {
      setLayouts(
        socket.raw.reduce(
          (prev, s, si) =>
            prev.concat(
              s.getCustomPageLayout().map((l, li) => ({
                name: l.name,
                sid: si,
                id: li,
              }))
            ),
          []
        )
      );
      setUpdateFlag(!updateFlag);
    }, 200);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag, socket]);

  return (
    <Box sx={{ padding: 1 }}>
      {pathname === "/grid" && (
        <>
          <SidebarButtonCollapse
            name="カスタムページ"
            icon={<WysiwygIcon />}
            visible={props.visible}
            contents={layouts.map((l) => ({
              name: l.name,
              icon: <WysiwygIcon />,
              selected: !!sidebarState.custom[`${l.sid}:${l.id}`],
              onClick: () =>
                setSidebarState((sidebarState) => {
                  sidebarState.custom[`${l.sid}:${l.id}`] =
                    !sidebarState.custom[`${l.sid}:${l.id}`];
                  return { ...sidebarState };
                }),
            }))}
          />
          <SidebarButton
            selected={sidebarState.logSelect}
            onClick={() =>
              setSidebarState((sidebarState) => ({
                ...sidebarState,
                logSelect: !sidebarState.logSelect,
              }))
            }
            name="グラフ表示"
            icon={<AutoGraphIcon />}
            visible={props.visible}
          />
          <SidebarButton
            selected={sidebarState.cameraSelect}
            onClick={() =>
              setSidebarState((sidebarState) => ({
                ...sidebarState,
                cameraSelect: !sidebarState.cameraSelect,
              }))
            }
            name="カメラ画像"
            icon={<PhotoCameraIcon />}
            visible={props.visible}
          />
          <SidebarButtonCollapse
            name="ログ出力"
            icon={<TerminalIcon />}
            visible={props.visible}
            contents={socket.raw.map((s, si) => ({
              selected: sidebarState.terminalLog[si],
              onClick: () =>
                setSidebarState((sidebarState) => {
                  sidebarState.terminalLog[si] = !sidebarState.terminalLog[si];
                  return { ...sidebarState };
                }),
              name: s.serverName,
              icon: <RouterOutLinedIcon />,
            }))}
          />
          <SidebarButtonCollapse
            name="シェル関数"
            icon={<KeyboardIcon />}
            visible={props.visible}
            contents={[
              {
                selected: sidebarState.shellFav,
                onClick: () =>
                  setSidebarState((sidebarState) => ({
                    ...sidebarState,
                    shellFav: !sidebarState.shellFav,
                  })),
                name: "登録済み",
                icon: <BookmarkIcon />,
              },
            ].concat(
              socket.raw.map((s, si) => ({
                selected: sidebarState.shell[si],
                onClick: () =>
                  setSidebarState((sidebarState) => {
                    sidebarState.shell[si] = !sidebarState.shell[si];
                    return { ...sidebarState };
                  }),
                name: s.serverName,
                icon: <RouterOutLinedIcon />,
              }))
            )}
          />
        </>
      )}
      <Divider sx={{ my: 1 }} />
      <SidebarRouter visible={props.visible} />
    </Box>
  );
}

interface MenuItem {
  icon: any;
  text: string;
  href: string;
  id?: number;
  sid?: number;
}

const default_items: MenuItem[] = [
  {
    icon: <RouterOutLinedIcon />,
    text: "接続",
    href: "/",
  },
  {
    icon: <DashboardOutlinedIcon />,
    text: "Grid-Layout",
    href: "/grid",
  },
  {
    icon: <AutoGraphIcon />,
    text: "グラフ表示",
    href: "/log",
  },
  {
    icon: <TerminalIcon />,
    text: "ログ出力",
    href: "/statistics",
  },
  {
    icon: <PhotoCameraIcon />,
    text: "カメラ画像",
    href: "/camera",
  },
  {
    icon: <KeyboardIcon />,
    text: "シェル関数",
    href: "/shell",
  },
  /*{
    icon: <SportsScoreIcon />,
    text: "本番画面",
    href: "/tournament",
  },
  {
    icon: <SportsEsportsIcon />,
    text: "テストラン",
    href: "/testrun",
  },*/
  {
    icon: <VideogameAssetIcon />,
    text: "JoyStick",
    href: "/joystick",
  },
  {
    icon: <SettingsIcon />,
    text: "接続情報",
    href: "/admin",
  },
];

type Props = {
  visible: boolean;
};
export function SidebarRouter(props: Props) {
  const { pathname, query } = useRouter();
  const socket = useSocket();
  const [items, setItems] = useState<MenuItem[]>([]);
  const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  useEffect(() => {
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 1000);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag]);
  useEffect(() => {
    setItems(
      socket.raw
        .reduce(
          (prev, s, si) =>
            prev.concat(
              s.getCustomPageLayout().map((e, i) => ({
                icon: <WysiwygIcon />,
                text: e.name,
                href: "/custom",
                id: i,
                sid: si,
              }))
            ),
          []
        )
        .concat(default_items)
    );
  }, [socket, updateFlag]);
  const [open, setOpen] = useState<boolean>(false);

  return (
    <>
      <SidebarButton
        onClick={() => setOpen(!open)}
        name="Router"
        icon={undefined}
        visible={props.visible}
        open={open}
        showOpenIcon={true}
      />
      <Collapse in={open && props.visible}>
        {items.map((item, i) => {
          return (
            <NextLink
              href={{
                pathname: item.href,
                query: { id: item.id, sid: item.sid },
              }}
              legacyBehavior
              key={i}
            >
              <ListItemButton
                dense
                selected={
                  pathname === item.href &&
                  (item.id == undefined ||
                    (query.id === item.id.toString() &&
                      query.sid === item.sid.toString()))
                }
                sx={{ pl: 4 }}
              >
                <ListItemIcon sx={{ minWidth: "40px" }}>
                  {item.icon}
                </ListItemIcon>
                <ListItemText primary={props.visible ? item.text : " "} />
              </ListItemButton>
            </NextLink>
          );
        })}
      </Collapse>
    </>
  );
}
