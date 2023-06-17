import "../styles/global.css";
import "react-grid-layout/css/styles.css";
import "react-resizable/css/styles.css";
import type { AppProps } from "next/app";
import CssBaseline from "@mui/material/CssBaseline";
import { useState } from "react";
import { createTheme, ThemeProvider } from "@mui/material/styles";
import Grid from "@mui/material/Grid";
import Header from "../components/header";
import Sidebar from "../components/sidebar";
import { SocketProvider } from "../components/socketContext";
import Snackbar from "../components/Message";
import {
  SidebarStateProvider,
  SidebarStateContext,
} from "../components/sidebarContext";
import { GamepadsProvider, GamepadsContext } from "react-gamepads";

const theme = createTheme({
  palette: {
    // mode: "dark"
  },
});

const GamepadsProvider_ = (props) => {
  if (typeof window === "undefined") {
    return (
      <GamepadsContext.Provider value={{ gamepads: {} }}>
        {props.children}
      </GamepadsContext.Provider>
    );
  } else {
    return <GamepadsProvider>{props.children}</GamepadsProvider>;
  }
};
export default function MyApp({ Component, pageProps }: AppProps) {
  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <GamepadsProvider_>
        <SocketProvider>
          <SidebarStateProvider>
            <SidebarStateContext.Consumer>
              {({ sidebarState, setSidebarState }) => (
                <div
                  style={{
                    position: "absolute",
                    top: 0,
                    bottom: 0,
                    left: 0,
                    right: 0,
                    display: "flex",
                    flexDirection: "column",
                  }}
                >
                  <div style={{ width: "100%", flexGrow: 0, flexShrink: 0 }}>
                    <Header
                      textInvert={() =>
                        setSidebarState((sidebarState) => ({
                          ...sidebarState,
                          sidebar: !sidebarState.sidebar,
                        }))
                      }
                    />
                  </div>
                  <div style={{ width: "100%", flexGrow: 1, flexShrink: 1 }}>
                    <Grid container sx={{ height: "100%" }}>
                      <Grid
                        item
                        xs={12}
                        sm={4}
                        md={3}
                        lg={2}
                        xl={1}
                        sx={{
                          display: sidebarState.sidebar ? "block" : "none",
                        }}
                      >
                        <Sidebar />
                      </Grid>
                      <Grid item xs sx={{ mx: 0.5, height: "100%" }}>
                        <Component {...pageProps} />
                      </Grid>
                    </Grid>
                  </div>
                </div>
              )}
            </SidebarStateContext.Consumer>
            <Snackbar />
          </SidebarStateProvider>
        </SocketProvider>
      </GamepadsProvider_>
    </ThemeProvider>
  );
}
