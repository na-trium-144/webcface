import { useContext, useState, useEffect, useRef } from "react";
import { useSocket } from "../components/socketContext";
import { GamepadsContext } from "react-gamepads";
import Grid from "@mui/material/Grid";
import InputLabel from "@mui/material/InputLabel";
import MenuItem from "@mui/material/MenuItem";
import FormControl from "@mui/material/FormControl";
import Button from "@mui/material/Button";
import ButtonGroup from "@mui/material/ButtonGroup";
import Select, { SelectChangeEvent } from "@mui/material/Select";
import Slider from "@mui/material/Slider";

export const GamepadView = (props: { gi: number }) => {
  const socket = useSocket();
  const { gamepads } = useContext(GamepadsContext);
  const { gi } = props;
  const connectedServer = socket.raw.findIndex(
    (s) => s.gamepadConnectIndex === gi
  );
  const [buttonName, setButtonName] = useState<string[]>([]);
  const [axisName, setAxisName] = useState<string[]>([]);
  const [buttonMap, setButtonMap] = useState<number[]>([]);
  const [axisMap, setAxisMap] = useState<{ id: number; inverse: boolean }[]>(
    []
  );
  const lsKey = "RC23WebCon-Gamepad";
  useEffect(() => {
    try {
      const lsItem = JSON.parse(window.localStorage.getItem(lsKey)) || {};
      if (lsItem[gamepads[gi].id]) {
        const si = socket.raw.findIndex(
          (s) => s.serverName === lsItem[gamepads[gi].id].server
        );
        const buttonMap = lsItem[gamepads[gi].id].b;
        const axisMap = lsItem[gamepads[gi].id].a;
        setButtonMap(buttonMap);
        setAxisMap(axisMap);
        for (let i = 0; i < buttonMap.length; i++) {
          socket.getByIndex(si).gamepadButtonMap.current[i] = buttonMap[i];
        }
        for (let i = 0; i < axisMap.length; i++) {
          socket.getByIndex(si).gamepadAxisMap.current[i] = axisMap[i];
        }
        setConnectedServer(si);
      }
    } catch {}
  }, [gi]);
  useEffect(() => {
    if (connectedServer >= 0) {
      setButtonName(socket.getByIndex(connectedServer).gamepadButtonSetting);
      setAxisName(socket.getByIndex(connectedServer).gamepadAxisSetting);
      setButtonMap((buttonMap) => {
        while (
          buttonMap.length <
          socket.getByIndex(connectedServer).gamepadButtonSetting.length
        ) {
          buttonMap.push(-1);
        }
        return buttonMap.slice();
      });
      setAxisMap((axisMap) => {
        while (
          axisMap.length <
          socket.getByIndex(connectedServer).gamepadAxisSetting.length
        ) {
          axisMap.push({ id: -1, inverse: false });
        }
        return axisMap.slice();
      });
    } else {
      setButtonName([]);
      setAxisName([]);
    }
  }, [connectedServer, socket]);
  const [modifyingId, setModifyingId] = useState<number>(-1);
  const [modifyingIsButton, setModifyingIsButton] = useState<boolean>(true);
  const [modifyingInput, setModifyingInput] = useState<boolean>(false);

  const setConnectedServer = (si: number | null) => {
    if (connectedServer != null && connectedServer >= 0) {
      socket.getByIndex(connectedServer).setGamepadConnectIndex(null);
    }
    if (si >= 0) {
      socket.getByIndex(si).setGamepadConnectIndex(gi);
    }
  };
  useEffect(() => {
    // modifyingId → ボタンが押される→ buttonMap設定, modifyingInput
    if (modifyingId >= 0 && !modifyingInput) {
      if (modifyingIsButton) {
        for (let i = 0; i < gamepads[gi].buttons.length; i++) {
          if (gamepads[gi].buttons[i].pressed) {
            buttonMap[modifyingId] = i;
            setButtonMap(buttonMap.slice());
            setModifyingInput(true);
            return;
          }
        }
      } else {
        for (let i = 0; i < gamepads[gi].axes.length; i++) {
          if (Math.abs(gamepads[gi].axes[i]) > 0.5) {
            axisMap[modifyingId] = { id: i, inverse: gamepads[gi].axes[i] < 0 };
            setAxisMap(axisMap.slice());
            setModifyingInput(true);
            return;
          }
        }
      }
    }
  }, [
    gamepads,
    gi,
    modifyingId,
    modifyingIsButton,
    axisMap,
    buttonMap,
    setAxisMap,
    setButtonMap,
    setModifyingInput,
  ]);
  useEffect(() => {
    // modifyingInput → ボタン離される → modifyingId=-1 → modifyingInput=false
    if (modifyingInput) {
      for (let i = 0; i < gamepads[gi].buttons.length; i++) {
        if (gamepads[gi].buttons[i].pressed) {
          return;
        }
      }
      for (let i = 0; i < gamepads[gi].axes.length; i++) {
        if (Math.abs(gamepads[gi].axes[i]) > 0.5) {
          return;
        }
      }
      setModifyingId(-1);
      setModifyingInput(false);

      if (connectedServer >= 0) {
        // 参照で直接書き換えるよ
        while (
          socket.getByIndex(connectedServer).gamepadButtonMap.current.length <
          buttonName.length
        ) {
          socket.getByIndex(connectedServer).gamepadButtonMap.current.push(-1);
        }
        while (
          socket.getByIndex(connectedServer).gamepadAxisMap.current.length <
          axisName.length
        ) {
          socket.getByIndex(connectedServer).gamepadAxisMap.current.push(-1);
        }
        for (let i = 0; i < buttonMap.length; i++) {
          socket.getByIndex(connectedServer).gamepadButtonMap.current[i] =
            buttonMap[i];
        }
        for (let i = 0; i < axisMap.length; i++) {
          socket.getByIndex(connectedServer).gamepadAxisMap.current[i] =
            axisMap[i];
        }
        let lsItem;
        try {
          lsItem = JSON.parse(window.localStorage.getItem(lsKey)) || {};
        } catch {}
        lsItem[gamepads[gi].id] = {
          server: socket.getByIndex(connectedServer).serverName,
          b: buttonMap,
          a: axisMap,
        };
        window.localStorage.setItem(lsKey, JSON.stringify(lsItem));
        console.log(3);
      }
    }
  }, [
    setModifyingId,
    modifyingInput,
    setModifyingInput,
    gamepads,
    gi,
    socket,
    connectedServer,
  ]);
  return (
    <>
      <Grid container alignItems="center">
        <Grid item>バックエンドに接続:</Grid>
        <Grid item xs>
          <FormControl fullWidth>
            <InputLabel id={`GamepadViweSelect${gi}`}>接続先</InputLabel>
            <Select
              labelId={`GamepadViweSelect${gi}`}
              value={connectedServer}
              label="接続先"
              onChange={(e) => {
                setConnectedServer(e.target.value);
              }}
              size="small"
            >
              <MenuItem value={-1}>なし</MenuItem>
              {socket.raw.map((s, si) => (
                <MenuItem value={si} key={si}>
                  {s.serverName}
                </MenuItem>
              ))}
            </Select>
          </FormControl>
        </Grid>
      </Grid>
      <div>
        <Button
          onClick={() => {
            setButtonMap(buttonMap.map(() => -1));
            setAxisMap(axisMap.map(() => -1));
          }}
          color="error"
          size="small"
        >
          割当をクリア
        </Button>
      </div>
      buttons:
      <div>
        {buttonMap.map((_, i) => (
          <span key={i}>
            <Button
              color={
                modifyingIsButton && modifyingId === i ? "warning" : "primary"
              }
              variant={
                modifyingIsButton && modifyingId === i
                  ? "contained"
                  : buttonMap[i] >= 0 &&
                    gamepads[gi].buttons[buttonMap[i]].pressed
                  ? "contained"
                  : "outlined"
              }
              size="small"
              onClick={() => {
                if (modifyingIsButton && modifyingId === i) {
                  setModifyingId(-1);
                } else {
                  setModifyingId(i);
                  setModifyingIsButton(true);
                }
              }}
            >
              {buttonName[i]}=
              {modifyingIsButton && modifyingId === i && !modifyingInput
                ? "割り当てるボタンを押してください"
                : buttonMap[i] >= 0
                ? buttonMap[i]
                : "割当なし"}
            </Button>
          </span>
        ))}
      </div>
      axes:
      <div>
        {axisMap.map((_, i) => (
          <span key={i}>
            <Button
              color={
                !modifyingIsButton && modifyingId === i
                  ? "warning"
                  : axisMap[i].id < 0 ||
                    Math.abs(gamepads[gi].axes[axisMap[i].id]) < 0.01 ||
                    gamepads[gi].axes[axisMap[i].id] < 0 == axisMap[i].inverse
                  ? "primary"
                  : "error"
              }
              variant={
                !modifyingIsButton && modifyingId === i
                  ? "contained"
                  : axisMap[i].id >= 0 &&
                    Math.abs(gamepads[gi].axes[axisMap[i].id]) > 0.5
                  ? "contained"
                  : "outlined"
              }
              size="small"
              onClick={() => {
                if (!modifyingIsButton && modifyingId === i) {
                  setModifyingId(-1);
                } else {
                  setModifyingId(i);
                  setModifyingIsButton(false);
                }
              }}
            >
              {axisName[i]}=
              {!modifyingIsButton && modifyingId === i && !modifyingInput
                ? "正の方向に入力してください"
                : axisMap[i].id >= 0
                ? axisMap[i].id.toString() +
                  (axisMap[i].inverse ? "(反転)" : "")
                : "割当なし"}
              <br />
              {(
                gamepads[gi].axes[axisMap[i].id] * (axisMap[i].inverse ? -1 : 1)
              ).toFixed(3)}
            </Button>
          </span>
        ))}
      </div>
    </>
  );
};
