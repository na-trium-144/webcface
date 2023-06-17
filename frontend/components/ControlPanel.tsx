import { Box, Grid, Button, Stack, Typography, Tooltip } from "@mui/material";
import { Stage, Layer, Rect, Text, Circle, Line } from "react-konva";
import {
  CustomPageComponentT,
  CustomPageValueT,
  CustomPageCallbackT,
  MultiSocketContextI,
  FromRobotDataT,
} from "../lib/global";
import { useSocket } from "../components/socketContext";
import { useState, useEffect, useRef } from "react";

type Props = {
  layout: CustomPageComponentT;
  sid: number;
};

const parseName = (name: string) => {
  // 他サーバーにある関数や変数の名前をパース
  const serverName = name.slice(0, name.indexOf(":"));
  const funcName = name.slice(name.indexOf(":") + 1);
  return [serverName, funcName];
};
const anyToString = (value: AnyValue) => {
  if (typeof value === "string") {
    return value;
  } else if (value != undefined && value != null) {
    return value.toString();
  } else {
    return "";
  }
};
const getValue = (
  value: CustomPageValueT,
  socket: MultiSocketContextI,
  sid: number
) => {
  if (value.value_name != undefined) {
    let data: FromRobotDataT;
    if (value.value_name.includes(":")) {
      const [serverName, funcName] = parseName(value.value_name);
      data = socket
        .get(serverName)
        .getFromRobotData()
        .find((m) => m.name === funcName);
    } else {
      data = socket
        .getByIndex(sid)
        .getFromRobotData()
        .find((m) => m.name === value.value_name);
    }
    if (data == undefined) {
      return `<'${value.value_name}' not found>`;
    } else {
      return data.lastdata;
    }
  } else {
    return value.value;
  }
};
const CustomPageComponent = (props: Props) => {
  const socket = useSocket();
  if (props.layout == undefined) {
    return <></>;
  }
  if (Array.isArray(props.layout)) {
    return (
      <>
        {props.layout.map(
          (l, li) =>
            li < props.layout.length && (
              <CustomPageComponent layout={l} sid={props.sid} key={li} />
            )
        )}
      </>
    );
  }
  switch (props.layout.type) {
    case "None":
      return <></>;
    case "Value":
      return (
        <Typography variant="body1" component="span" sx={{ m: 0.5 }}>
          {anyToString(getValue(props.layout.value, socket, props.sid))}
        </Typography>
      );
    case "Vector":
      return (
        <Grid container spacing={1} alignItems="center" sx={{ m: 0.5 }}>
          {props.layout.children.map((c, i) => (
            <Grid item key={i}>
              <CustomPageComponent layout={c} sid={props.sid} />
            </Grid>
          ))}
        </Grid>
      );
    case "Stack":
      return (
        <Stack spacing={1} sx={{ m: 0.5 }}>
          {props.layout.children.map((c, i) => (
            <Box key={i}>
              <CustomPageComponent layout={c} sid={props.sid} />
            </Box>
          ))}
        </Stack>
      );
    case "Button":
      return (
        <ControlButton
          displayName={props.layout.display_name}
          callback={props.layout.callback}
          sid={props.sid}
          color={props.layout.color}
        />
      );
    case "Drawing":
      return (
        <Drawing
          sid={props.sid}
          width={props.layout.width}
          height={props.layout.height}
          children={props.layout.children}
        />
      );
    case "br":
      return <br />;
  }
};

const Drawing = (props: {
  sid: number;
  width: CustomPageValueT;
  height: CustomPageValueT;
  children: CustomPageComponentT[][];
}) => {
  const socket = useSocket();
  const layoutWidth = getValue(props.width, socket, props.sid);
  const layoutHeight = getValue(props.height, socket, props.sid);
  const [width, setWidth] = useState<number>(layoutWidth);
  const [zoom, setZoom] = useState<number>(1);
  const canvasDiv = useRef<HTMLCanvasElement>(null);
  const [updateFlag, setUpdateFlag] = useState<boolean>(false);
  useEffect(() => {
    const i = setTimeout(() => {
      if (width !== canvasDiv.current.clientWidth) {
        setWidth(canvasDiv.current.clientWidth);
        setZoom(canvasDiv.current.clientWidth / layoutWidth);
      }
      setUpdateFlag(!updateFlag);
    }, 100);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag, width, setWidth, layoutWidth]);
  return (
    <div
      style={{
        width: "100%",
        overflow: "hidden",
      }}
      ref={canvasDiv}
    >
      <Stage width={width} height={layoutHeight * zoom}>
        {props.children.map((l, li) => (
          <Layer key={li}>
            {l.map((c, ci) => (
              <DrawingComponent
                key={ci}
                component={c}
                sid={props.sid}
                zoom={zoom}
              />
            ))}
          </Layer>
        ))}
      </Stage>
    </div>
  );
};
const DrawingComponent = (props: {
  component: CustomPageComponentT;
  sid: number;
  zoom: number;
}) => {
  const socket = useSocket();
  const onClick = () => {
    if (props.component.on_click != undefined) {
      runCustomCallback(props.component.on_click, socket, props.sid);
    }
  };
  switch (props.component.type) {
    case "Line": {
      const x = getValue(props.component.x, socket, props.sid) * props.zoom;
      const y = getValue(props.component.y, socket, props.sid) * props.zoom;
      const x2 = getValue(props.component.x2, socket, props.sid) * props.zoom;
      const y2 = getValue(props.component.y2, socket, props.sid) * props.zoom;
      const color = getValue(props.component.color, socket, props.sid);
      return (
        <Line
          x={x}
          y={y}
          points={[0, 0, x2 - x, y2 - y]}
          stroke={color}
          onClick={onClick}
        />
      );
    }
    case "Rect": {
      const x = getValue(props.component.x, socket, props.sid) * props.zoom;
      const y = getValue(props.component.y, socket, props.sid) * props.zoom;
      const x2 = getValue(props.component.x2, socket, props.sid) * props.zoom;
      const y2 = getValue(props.component.y2, socket, props.sid) * props.zoom;
      const color = getValue(props.component.color, socket, props.sid);
      return (
        <Rect
          x={x}
          y={y}
          width={x2 - x}
          height={y2 - y}
          fill={color}
          onClick={onClick}
        />
      );
    }
    case "Circle": {
      const x = getValue(props.component.x, socket, props.sid) * props.zoom;
      const y = getValue(props.component.y, socket, props.sid) * props.zoom;
      const r = getValue(props.component.r, socket, props.sid) * props.zoom;
      const color = getValue(props.component.color, socket, props.sid);
      return <Circle x={x} y={y} radius={r} fill={color} onClick={onClick} />;
    }
  }
};

const runCustomCallback = (
  callback: CustomPageCallbackT,
  socket: MultiSocketContextI,
  sid: number
) => {
  let arg_values: AnyValue[] | undefined = undefined;
  if (callback.args != undefined) {
    arg_values = callback.args.map((a) => getValue(a, socket, sid));
  }
  if (callback.callback_name.includes(":")) {
    const [serverName, funcName] = parseName(callback.callback_name);
    socket.get(serverName).runCallback(funcName, arg_values);
  } else {
    socket.getByIndex(sid).runCallback(callback.callback_name, arg_values);
  }
};
interface ButtonProps {
  displayName: CustomPageValueT;
  callback: CustomPageCallbackT;
  color?: CustomPageValueT;
  sid: number;
}
const ControlButton = (props: ButtonProps) => {
  const socket = useSocket();
  const [exists, setExists] = useState<boolean>(false);
  useEffect(() => {
    if (props.callback.callback_name.includes(":")) {
      const [serverName, funcName] = parseName(props.callback.callback_name);
      const exists =
        socket
          .get(serverName)
          .functionSetting.findIndex((f) => f.name === funcName) >= 0;
      setExists(exists);
    } else {
      const exists =
        socket
          .getByIndex(props.sid)
          .functionSetting.findIndex(
            (f) => f.name === props.callback.callback_name
          ) >= 0;
      setExists(exists);
    }
  }, [socket, setExists, props]);
  const handler = () => {
    runCustomCallback(props.callback, socket, props.sid);
  };
  return (
    <Tooltip
      title={
        exists
          ? `${props.callback.callback_name}(${
              props.callback.args
                ? props.callback.args
                    .map((a) => getValue(a, socket, props.sid))
                    .join(", ")
                : ""
            })`
          : `'${props.callback.callback_name}' not found`
      }
    >
      <span>
        <Button
          variant="contained"
          size="large"
          onClick={handler}
          sx={{ textTransform: "none", m: 0.5 }}
          disabled={!exists}
          color={
            props.color != undefined
              ? getValue(props.color, socket, props.sid)
              : "primary"
          }
        >
          {anyToString(getValue(props.displayName, socket, props.sid))}
        </Button>
      </span>
    </Tooltip>
  );
};

export default function TestRunPage(props: Props) {
  return <CustomPageComponent layout={props.layout} sid={props.sid} />;
}
