import { Box, Tabs, Tab } from "@mui/material";
import RouterTwoToneIcon from "@mui/icons-material/RouterTwoTone";
import RouterOutLinedIcon from "@mui/icons-material/RouterOutlined";
import * as React from "react";

export default function ServerSelectTab(props: {
  tabPage: number;
  setTabPage: (tabPage: number) => void;
  serverNames: string[];
}) {
  const { tabPage, setTabPage, serverNames } = props;
  return (
    <Box sx={{ borderBottom: 1, borderColor: "divider" }}>
      <Tabs
        value={tabPage}
        onChange={(event: React.SyntheticEvent, newValue: number) =>
          setTabPage(newValue)
        }
      >
        {serverNames.map((sn, i) => (
          <Tab
            key={i}
            icon={
              tabPage === i ? <RouterTwoToneIcon /> : <RouterOutLinedIcon />
            }
            iconPosition="start"
            label={sn}
            sx={{ textTransform: "none" }}
          />
        ))}
      </Tabs>
    </Box>
  );
}
