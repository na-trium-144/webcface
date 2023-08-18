import { Card } from "./card";
import { Value } from "webcface";
import { useState, useEffect, useRef } from "react";
import { WebglPlot, WebglLine, ColorRGBA } from "webgl-plot";

interface Props {
  value: Value;
}
export function ValueCard(props: Props) {
  const canvasMain = useRef<HTMLCanvasElement>(null);
  const canvasDiv = useRef<HTMLCanvasElement>(null);
  const data = useRef<number[]>([]);
  const lastUpdate = useRef<Date>(new Date());

  useEffect(() => {
    const onValueChange = () => {
      const val = props.value.tryGet();
      if (val != null) {
        const now = new Date();
        const timeDiff = now.getTime() - lastUpdate.current.getTime();
        lastUpdate.current = now;
        for (let t = 0; t < timeDiff; t++) {
          data.current.push(val);
        }
      }
    };
    onValueChange();
    props.value.on(onValueChange);
    return () => props.value.off(onValueChange);
  }, [props.value]);

  useEffect(() => {
    let webglp: WebglPlot | null = null;
    const line: WebglLine = new WebglLine(new ColorRGBA(0, 0.6, 0, 1), 5000);
    line.arrangeX();

    if (canvasMain.current) {
      let id = 0;
      let renderPlot = () => {
        if (
          webglp == null ||
          canvasMain.current.width != canvasDiv.current.clientWidth ||
          canvasMain.current.height !== canvasDiv.current.clientHeight
        ) {
          canvasMain.current.width = canvasDiv.current.clientWidth;
          canvasMain.current.height = canvasDiv.current.clientHeight;

          webglp = new WebglPlot(canvasMain.current);
          webglp.addLine(line);
        }

        let maxY: number | null = null;
        let minY: number | null = null;
        let midY = 0;
        for (let i = 0; i < line.numPoints; i++) {
          if (data.current.length - line.numPoints + i >= 0) {
            const val = data.current[data.current.length - line.numPoints + i];
            if (maxY == null || maxY < val) {
              maxY = val;
            }
            if (minY == null || minY > val) {
              minY = val;
            }
            if (maxY != null && minY != null) {
              midY = (maxY + minY) / 2;
              if (maxY === minY) {
                maxY += 1;
                minY -= 1;
              }
            }
            line.setY(i, val);
          }
        }
        if (maxY != null && minY != null) {
          line.offsetY = -midY / (maxY - midY);
          line.scaleY = 1 / (maxY - midY);
        }
        id = requestAnimationFrame(renderPlot);
        webglp.update();
      };
      id = requestAnimationFrame(renderPlot);

      return () => {
        renderPlot = () => undefined;
        cancelAnimationFrame(id);
      };
    }
  }, []);

  return (
    <Card title={`${props.value.member.name}:${props.value.name}`}>
      <div className="flex flex-col h-full">
        <div className="flex-1 w-full" ref={canvasDiv}>
          <canvas ref={canvasMain} />
        </div>
        <div className="flex-none">
          <input
            type="range"
            className="cursor-pointer w-full"
          />
        </div>
      </div>
    </Card>
  );
}
