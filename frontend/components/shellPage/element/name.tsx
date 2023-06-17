import {Typography} from "@mui/material";

export default function Name({children}) {
  return (
    <Typography sx={{ wordBreak: "break-word" }} variant="h6">
      {children}
    </Typography>
  );
}
