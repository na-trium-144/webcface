import { useState, useEffect, forwardRef, useRef } from "react";
import Snackbar from "@mui/material/Snackbar";
import MuiAlert, { AlertProps, AlertColor } from "@mui/material/Alert";
import { useSocket } from "./socketContext";

const Alert = forwardRef<HTMLDivElement, AlertProps>(function Alert(
  props,
  ref
) {
  return <MuiAlert elevation={6} ref={ref} variant="filled" {...props} />;
});

export default function CustomizedSnackbars() {
  const [open, setOpen] = useState<boolean>(false);
  const [msg, setMsg] = useState<string>("");
  const [severity, setSeverity] = useState<string>("success");
  const [autoHide, setAutoHide] = useState<number | null>(null);
  const socket = useSocket();
  useEffect(() => {
    const i = setInterval(() => {
      for (const s of socket.raw) {
        const errors = s.getErrorMessage();
        for (let ei = 0; ei < errors.length; ei++) {
          if (errors[ei].message === "") {
            continue;
          }
          setAutoHide(null);
          if (errors[ei].message === "Connecting") {
            setSeverity("info");
          } else if (errors[ei].message === "Running") {
            setSeverity("info");
          } else if (errors[ei].message === "Done") {
            setSeverity("success");
            setAutoHide(1500);
          } else {
            setSeverity("error");
          }
          setMsg(`[${s.serverName}:${errors[ei].func}(${errors[ei].args.map((a) => JSON.stringify(a)).join(", ")})] ${errors[ei].message}`);
          setOpen(true);
          errors[ei].message = "";
          return;
        }
      }
    }, 100);
    return () => clearInterval(i);
  }, [socket, setOpen, setMsg, setSeverity]);
  const handleClose = (_?: React.SyntheticEvent | Event, reason?: string) => {
    if (reason === "clickaway") {
      return;
    }
    setOpen(false);
  };

  const snackbar = (
    <Snackbar open={open} onClose={handleClose} autoHideDuration={autoHide}>
      <Alert
        onClose={handleClose}
        severity={severity as AlertColor}
        sx={{ width: "100%" }}
      >
        {msg}
      </Alert>
    </Snackbar>
  );

  return snackbar;
}
