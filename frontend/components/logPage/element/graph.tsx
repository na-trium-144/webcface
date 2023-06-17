import { Extend } from "@/components/function/propExtend";
// import * as merge from "deepmerge";
// https://github.com/apexcharts/react-apexcharts/issues/240
// import dynamic from "next/dynamic";

import { useEffect, useRef, useState } from "react";
import { WebglPlot, WebglLine, ColorRGBA } from "webgl-plot";
import { Typography } from "@mui/material";
/**
 * 1つの折れ線グラフの折れ線に該当するデータセット
 */
type DataSet = {
  //その折れ線の名称
  name?: string;
  data: number;
  timestamp: Date;
};

type MultiLineProps = {
  title?: string;
  // ApexChart にそのまま渡される
  // options?: any;
  // データセットの配列
  series: Array<DataSet>;
};
/**
 * 複数の折れ線を描画するグラフ
 *
 * @param {MultiLineProps} props
 */
export function MultiLine(props: MultiLineProps) {
  const canvasMain = useRef<HTMLCanvasElement>(null);
  const canvasDiv = useRef<HTMLCanvasElement>(null);

  const webglp = useRef<WebglPlot>(null);
  const line = useRef<WebglLine[]>([]);
  // const [maxY, setMaxY] = useState<number>();
  // const [minY, setMinY] = useState<number>();
  const maxY = useRef<number | null>(null);
  const minY = useRef<number | null>(null);
  const midY = useRef<number>(0);
  const lastTimestamps = useRef<Date[]>([]);
  const numX = 1000;
  // const resolution = 1; // millisec

  const onResize = () => {
    // const devicePixelRatio = window.devicePixelRatio || 1;
    canvasMain.current.width =
      canvasDiv.current.clientWidth /** devicePixelRatio*/;
    canvasMain.current.height =
      canvasDiv.current.clientHeight /** devicePixelRatio*/;

    webglp.current = new WebglPlot(canvasMain.current);

    lastTimestamps.current = [];
    line.current = [];
    for (let i = 0; i < props.series.length; i++) {
      line.current.push(new WebglLine(new ColorRGBA(0, 0.5, 1, 1), numX));
      webglp.current.addLine(line.current[i]);
      line.current[i].arrangeX();
      lastTimestamps.current.push(new Date());
    }
    // setMaxY(undefined);
    // setMinY(undefined);
  };
  useEffect(() => {
    if (canvasMain.current) {
      onResize();
    }
  }, [props.series.length, props.title]);
  useEffect(() => {
    if(canvasMain.current.width != canvasDiv.current.clientWidth || canvasMain.current.height !== canvasDiv.current.clientHeight){
      console.log("resize");
      onResize();
    }
    let id = 0;
    let renderPlot = () => {
      // setMaxY((maxY) => {
      // setMinY((minY) => {
      for (let i = 0; i < line.current.length; i++) {
        const dataSet = props.series[i];
        if (
          lastTimestamps.current[i].getTime() !== dataSet.timestamp.getTime()
        ) {
          for (const v of dataSet.data) {
            if (typeof v === "number") {
              if (maxY.current == null || maxY.current < v) {
                maxY.current = v;
              }
              if (minY.current == null || minY.current > v) {
                minY.current = v;
              }
            }
          }
          if (maxY.current != null) {
            midY.current = (maxY.current + minY.current) / 2;
            if (maxY.current == minY.current) {
              maxY.current += 1;
              minY.current -= 1;
            }
          }
          // for (
          //   let t = lastTimestamps.current[i].getTime();
          //   t < dataSet.timestamp.getTime();
          //   t += resolution
          // ) {
          // line.current[i].shiftAdd([dataSet.data]);
          // }
          // line.current[i].shiftAdd(dataSet.data);
          // console.log(dataSet.data)
          for (let j = 0; j < dataSet.data.length; j++) {
            line.current[i].setY(
              numX - dataSet.data.length + j,
              dataSet.data[j]
            );
          }
          lastTimestamps.current[i] = new Date(dataSet.timestamp);
          line.current[i].offsetY =
            -midY.current / (maxY.current - midY.current);
          line.current[i].scaleY = 1 / (maxY.current - midY.current);
        }
      }
      id = requestAnimationFrame(renderPlot);
      webglp.current.update();

      // return minY;
      // });
      // return maxY;
      // });
    };
    id = requestAnimationFrame(renderPlot);

    return () => {
      renderPlot = () => {
        return;
      };
      cancelAnimationFrame(id);
    };
  }, [props.series]);

  const canvasStyle = {
    // width: "100%",
    // height: "100%",
  };

  return (
    <>
      <Typography
        variant="caption"
        display="block"
        sx={{ flexBasis: "auto", flexGrow: 0, flexShrink: 0 }}
      >
        {maxY.current}
      </Typography>
      <div
        style={{
          flexBasis: "auto",
          flexGrow: 1,
          flexShrink: 1,
          overflow: "hidden",
        }}
        ref={canvasDiv}
      >
        <canvas style={canvasStyle} ref={canvasMain} />
      </div>
      <Typography
        variant="caption"
        display="block"
        sx={{ flexBasis: "auto", flexGrow: 0, flexShrink: 0 }}
      >
        {minY.current}
      </Typography>
    </>
  );
}

type SingleLineOwnProps = {
  series: DataSet;
};

type SingleLineProps = Extend<MultiLineProps, SingleLineOwnProps>;
/**
 * 1つの折れ線のみを表示するグラフ
 *
 * @param {SingleLineProps} props
 */
export function SingleLine(props: SingleLineProps) {
  const { series, ...other } = props;
  return <MultiLine series={[series]} {...other} />;
}
