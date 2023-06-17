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
import { SidebarStateProvider } from "../components/sidebarContext";

const theme = createTheme({
  palette: {
    // mode: "dark"
  },
});

export default function MyApp({ Component, pageProps }: AppProps) {
  const [textVisible, setTextVisible] = useState(true);
  const textInvert = () => {
    setTextVisible(!textVisible);
  };
  return (
    <ThemeProvider theme={theme}>
      <CssBaseline />
      <SocketProvider>
        <SidebarStateProvider>
          <Grid container spacing={2}>
            <Grid item xs={12}>
              <Header textInvert={textInvert} />
            </Grid>
            <Grid
              item
              xs={textVisible ? 5 : 2.5}
              sm={textVisible ? 3 : 2}
              md={textVisible ? 2.5 : 1.5}
              lg={textVisible ? 2 : 1}
            >
              <Sidebar visible={textVisible} />
            </Grid>
            <Grid item xs sx={{ mx: 0.5 }}>
              <Component {...pageProps} />
            </Grid>
          </Grid>
        </SidebarStateProvider>
      </SocketProvider>
    </ThemeProvider>
  );
}
