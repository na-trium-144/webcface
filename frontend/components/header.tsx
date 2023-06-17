import { AppBar, IconButton, Toolbar, Typography, Chip } from "@mui/material";
import MenuIcon from "@mui/icons-material/Menu";
import ErrorOutlineIcon from "@mui/icons-material/ErrorOutline";
import CheckIcon from "@mui/icons-material/Check";
import { useSocket } from "./socketContext";
import Link from "next/link";
import version from "../lib/version";

// 参考記事: https://mui.com/material-ui/react-app-bar/

type Props = {
  textInvert: () => void;
};
export default function Header(props: Props) {
  const socket = useSocket();

  return (
    <AppBar position="static">
      <Toolbar variant="dense">
        <IconButton
          edge="start"
          color="inherit"
          area-label="menu"
          sx={{ mr: 2 }}
          onClick={() => {
            props.textInvert();
            console.log("menu onclick");
            // console.log(props.textVisible);
          }}
        >
          <MenuIcon />
        </IconButton>
        {/* <SwipeableDrawer
          anchor="left"
          open={showDrawer}
          onClose={() => setShowDrawer(false)}
          onOpen={() => {}}
        >
          <Sidebar/>
        </SwipeableDrawer> */}
        <Typography variant="h6" color="inherit" component="div" sx={{ m: 1 }}>
          <Link href="/" style={{ color: "white", textDecoration: "none" }}>
            WebCFace
            <Typography
              variant="subtitle2"
              color="inherit"
              component="span"
              sx={{ m: 1 }}
            >
              ver. {version}
            </Typography>
          </Link>
        </Typography>
        {socket.raw.map((s, i) => (
          <Chip
            icon={s.isConnected ? <CheckIcon /> : <ErrorOutlineIcon />}
            label={s.serverName || s.socketUrl}
            variant="filled"
            color={s.isConnected ? "success" : "error"}
            key={i}
            sx={{ mr: 1 }}
          />
        ))}
      </Toolbar>
    </AppBar>
  );
}
