import * as React from "react";
import Snackbar from "@mui/material/Snackbar";
import MuiAlert, { AlertProps, AlertColor } from "@mui/material/Alert";

const Alert = React.forwardRef<HTMLDivElement, AlertProps>(function Alert(
  props,
  ref
) {
  return <MuiAlert elevation={6} ref={ref} variant="filled" {...props} />;
});

export default function CustomizedSnackbars() {
  const [open, setOpen] = React.useState(false);
  const [msg, setMsg] = React.useState("");
  const [severity, setSeverity] = React.useState("success");
  const opener = (msg: string, severity: string) => {
    setMsg(msg);
    setSeverity(severity);
    setOpen(true);
  };

  const handleClose = (_?: React.SyntheticEvent | Event, reason?: string) => {
    if (reason === "clickaway") {
      return;
    }
    setOpen(false);
  };

  const snackbar = (
    <Snackbar open={open} autoHideDuration={2000} onClose={handleClose}>
      <Alert
        onClose={handleClose}
        severity={severity as AlertColor}
        sx={{ width: "100%" }}
      >
        {msg}
      </Alert>
    </Snackbar>
  );

  return [snackbar, opener];
}
