import React, { useRouter } from "next/router";
import {
  CircularProgress,
  Typography,
  Grid,
  Paper,
  Button,
  Divider,
  TextField,
} from "@mui/material";
import { useEffect, useState } from "react";
import { useSocket } from "../components/socketContext";

export default function LogPage() {
  const socket = useSocket();
  const { isConnected, socketUrl } = socket.getByIndex(0);
  const router = useRouter();
  const [socketUrlInput, setSocketUrlInput] = useState<string>("");

  useEffect(() => {
    if (isConnected) {
      void router.push("/grid");
    }
  }, [isConnected, router]);

  useEffect(() => {
    setSocketUrlInput(socketUrl);
  }, [socketUrl]);

  return (
    <>
      <Grid
        container
        spacing={0}
        direction="column"
        alignItems="center"
        justifyContent="center"
        style={{ minHeight: "100vh" }}
      >
        <Grid item xs={3}>
          <Paper
            elevation={3}
            sx={{ m: 2, p: 2, alignItems: "center", maxWidth: "500px" }}
          >
            <Typography variant="h6" component="p">
              <CircularProgress />
              Connection : {isConnected ? "true" : "false"}
            </Typography>
            <Divider sx={{ m: 2 }} />
            <TextField
              fullWidth
              label="url"
              value={socketUrlInput}
              onChange={(e: React.ChangeEvent<HTMLInputElement>) => {
                setSocketUrlInput(e.target.value);
              }}
            />
            <Button
              variant="contained"
              fullWidth={true}
              type="submit"
              onClick={() => {
                // reconnect(socketUrlInput);
                socket.setByIndex(0, {...socket.getByIndex(0), socketUrl: socketUrlInput});
              }}
            >
              再接続
            </Button>
          </Paper>
        </Grid>
      </Grid>
    </>
  );
}
