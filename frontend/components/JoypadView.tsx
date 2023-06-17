import { JoypadData } from '../lib/useJoypad'
import { Chip } from "@mui/material";

// 参考記事: https://mui.com/material-ui/react-app-bar/
type Props = {
  joypad: JoypadData
}

const BTNS = ["L", "R", "T", "B", "△", "○", "☓", "□", "L1", "R1", "L2", "R2", "L3", "R3", "SEL", "START"];
const DOT_SIZE = 6;
let dots = {};

export default function JoypadView(props: Props) {
  const { joypad } = props;

  const drawJoystick = (values: number[], left: boolean) => {
    if (typeof window !== 'object') return;

    const id = (left ? 'left' : 'right');
    const canvas = document.getElementById(id + '-joystick') as any;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (dots[id]) {
      ctx.clearRect(dots[id].x - 1, dots[id].y - 1, DOT_SIZE + 2, DOT_SIZE + 2);
    }
    const x = Math.ceil(canvas.width / 2 * (1 + values[0] / 100));
    const y = Math.ceil(canvas.height / 2 * (1 + values[1] / 100));
    const rect = { x: x - DOT_SIZE / 2, y: y - DOT_SIZE / 2 };
    ctx.fillStyle = 'cyan';
    ctx.fillRect(rect.x, rect.y, DOT_SIZE, DOT_SIZE);
    ctx.strokeStyle = 'black';
    ctx.strokeRect(rect.x, rect.y, DOT_SIZE, DOT_SIZE);
    dots[id] = rect;
  }

  const draw = () => {
    drawJoystick([joypad.axis[0], joypad.axis[1]], true);
    drawJoystick([joypad.axis[2], joypad.axis[3]], false);
  }

  draw();

  return (
    <>
      <canvas id='left-joystick' width='100' height='100'></canvas>
      <canvas id='right-joystick' width='100' height='100'></canvas>
      <p id="joypad-id">Joypad ID = -1</p>
      <div>
        {
          joypad.buttons.map((b, i) => {
            if (b) return <Chip key={i} label={BTNS[i]} color="primary" />;
            else return <Chip key={i} label={BTNS[i]} />;
          })
        }
      </div>
    </>
  );
}


