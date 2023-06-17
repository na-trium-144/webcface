import { Box } from "@mui/material";
import { useState, useEffect } from "react";
import { useSocketFromRobot } from "./socketContext";
import { myPlot, Colors, Color } from "../lib/drawer";
import { useWindowSize } from "./useWindowSize";

/**
 * ロボット、フィールド、リングを含めた表示を担当する
 */
// export default function Field() {
//   const fieldSize = 12000;
//   return (
//     <Box position="relative" height="100%" >
//       <FieldImage />
//       <Robot teamColor="red" robotKind="er" fieldSize={fieldSize} position={robot_pos} size={500}/>
//       <Robot teamColor="red" robotKind="rr" fieldSize={fieldSize} position={robot_pos} />
//     </Box>
//   );
// }

const field_data = [
  // 大枠
  { p0: [0, 0], p1: [6000, 12000], col: Colors.dark_red },
  { p0: [6000, 0], p1: [12000, 12000], col: Colors.dark_blue },

  { p0: [0, 0], p1: [50, 12000], col: Colors.black },
  { p0: [0, 0], p1: [12000, 50], col: Colors.black },
  { p0: [11950, 0], p1: [12000, 12000], col: Colors.black },
  { p0: [0, 11950], p1: [12000, 12000], col: Colors.black },
  { p0: [6000, 0], p1: [6025, 12000], col: Colors.black },

  // 四隅
  { p0: [50, 50], p1: [550, 550], col: Colors.dark_gray },
  { p0: [11450, 50], p1: [11950, 550], col: Colors.dark_gray },
  { p0: [50, 11450], p1: [550, 11950], col: Colors.dark_gray },
  { p0: [11450, 11450], p1: [11950, 11950], col: Colors.dark_gray },

  { p0: [6000, 50], p1: [6500, 550], col: Colors.dark_yellow },
  { p0: [5500, 11450], p1: [6000, 11950], col: Colors.dark_yellow },

  { p0: [4500, 4500], p1: [7500, 7500], col: Colors.dark_red2 },
  { p0: [4000, 5500], p1: [4500, 6500], col: Colors.gray },
  { p0: [7500, 5500], p1: [8000, 6500], col: Colors.gray },

  { p0: [50, 5250], p1: [1050, 6750], col: Colors.red },
  { p0: [10500, 5250], p1: [11950, 6750], col: Colors.blue },

  // 堀エリア
  { p0: [1975, 1975], p1: [2575, 10025], col: Colors.blue },
  { p0: [1975, 1975], p1: [10025, 2575], col: Colors.blue },
  { p0: [1975, 9425], p1: [10025, 10025], col: Colors.blue },
  { p0: [9425, 1975], p1: [10025, 10025], col: Colors.blue },

  { p0: [5025, 9425], p1: [6000, 10025], col: Colors.dark_blue2 },
  { p0: [6000, 1975], p1: [6975, 2575], col: Colors.dark_blue2 },
];

const poles = [
  [6000, 6000],
  [4700, 4700],
  [7300, 4700],
  [4700, 7300],
  [7300, 7300],
  [2800, 6000],
  [9200, 6000],
  [2600, 9200],
  [2600, 2600],
  [9200, 9200],
  [9200, 2600],
];

export default function Field() {
  const { getFromRobotData } = useSocketFromRobot();
  const [drawer, setDrawer] = useState(null);
  const window_size = useWindowSize();

  const draw_field = () => {
    for (const f of field_data) {
      drawer.rectangle_top_bottom(f.p0, f.p1, f.col);
    }
    for (const f of poles) {
      drawer.circle(f, 100, Colors.yellow);
    }
  };

  const draw_robot = (robot_pos: Array<number>) => {
    if (drawer == null) return;
    const xy = [robot_pos[0], robot_pos[1]];
    drawer.wired_circle(xy, 500, [255, 0, 200], 30);
    const r = 1000;
    const theta = robot_pos[2];
    const to = [xy[0] + r * Math.cos(theta), xy[1] + r * Math.sin(theta)];
    drawer.line(xy, to, [255, 0, 200], 30);
  };

  const draw_lrf_data = (lrf_data: Array<number>, col: Color) => {
    if (drawer == null) return;
    const pc_size = 50;
    for (const l of lrf_data) {
      drawer.point(l, col, pc_size);
    }
  };

  const [updateFlag, setUpdateFlag] = useState(false);
  useEffect(() => {
    if (drawer == null) return;

    const from_robot = getFromRobotData();

    const get_robot_pos = () => {
      const idx = from_robot.findIndex((el) => el.name === "robot_pos");
      if (idx < 0) return [];
      const r = from_robot[idx].data;
      if (r.length < 1) return [];
      const pos = r[r.length - 1];
      return pos;
    };

    const get_lrf_data = () => {
      const idx = from_robot.findIndex((el) => el.name === "lrf");
      if (idx < 0) return [];
      const r = from_robot[idx].data;
      if (r.length < 1) return [];
      return r;
    };

    const robot_pos = get_robot_pos();
    const lrf_data = get_lrf_data();

    drawer.clear_buffer();
    draw_field();
    draw_robot(robot_pos);
    draw_lrf_data(lrf_data, Colors.yellow);
    drawer.update_buffers();
    drawer.draw();

    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 30);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag]);

  useEffect(() => {
    const canvas = document.getElementById("my_canvas");
    const wrapper = document.getElementById("canvas_wrapper");

    const width = wrapper.clientWidth; //document.width is obsolete
    const height = wrapper.clientHeight; //document.height is obsolete
    console.log(width, height);
    canvas.style.width = width.toString() + "px";
    canvas.style.height = width.toString() + "px";

    const A = new myPlot("my_canvas");
    A.set_field_size(12000, 12000);
    A.create_shader();
    setDrawer(A);
  }, [window_size]);

  return (
    <Box
      id="canvas_wrapper"
      sx={{ width: "100%", height: "100%", background: "red" }}
    >
      <canvas width="1000" height="1000" id="my_canvas"></canvas>
    </Box>
  );
}
