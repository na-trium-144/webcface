import { useState, useEffect, forwardRef, useRef } from "react";
import { useSocket } from "./socketContext";
import { parseName } from "../lib/global";
import { CustomPage } from "../pages/custom";
import Button from "@mui/material/Button";
import Dialog from "@mui/material/Dialog";
import DialogActions from "@mui/material/DialogActions";
import DialogContent from "@mui/material/DialogContent";
import DialogContentText from "@mui/material/DialogContentText";
import DialogTitle from "@mui/material/DialogTitle";

export default function Alert(props: {}) {
  const [open, setOpen] = useState<boolean>(false);
  const [alertName, setAlertName] = useState<string>("");
  const [id, setId] = useState<number>(-1);
  const [sid, setSid] = useState<number>(-1);
  const socket = useSocket();
  useEffect(() => {
    if (!open) {
      const i = setInterval(() => {
        for (let si = 0; si < socket.raw.length; si++) {
          const s = socket.getByIndex(si);
          const alert = s.getAlert();
          if (alert) {
            setOpen(true);
            setAlertName(alert);
            if (alert.includes(":")) {
              const [serverName, funcName] = parseName(alert);
              const sid = socket.raw.findIndex(
                (s) => s.serverName === serverName
              );
              const id = socket
                .getByIndex(sid)
                .getCustomPageLayout()
                .findIndex((l) => l.name === funcName);
              setId(id);
              setSid(sid);
            } else {
              setSid(si);
              const id = s
                .getCustomPageLayout()
                .findIndex((l) => l.name === alert);
              setId(id);
            }
            s.clearAlert();
            return;
          }
        }
      }, 100);
      return () => clearInterval(i);
    }
  }, [socket, open, setOpen, setId, setSid, setAlertName]);

  return (
    <Dialog open={open} onClose={() => {}}>
      <DialogTitle>{alertName}</DialogTitle>
      <DialogContent>{open && <CustomPage sid={sid} id={id} />}</DialogContent>
      <DialogActions>
        <Button onClick={() => setOpen(false)}>Close</Button>
      </DialogActions>
    </Dialog>
  );
}
