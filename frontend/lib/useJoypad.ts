import { useState, useEffect } from 'react';

export type JoypadData = {
  buttons: number[],
  axis: number[],
  name: string,
  connected: boolean,
}

export type JoypadUpdateHandlerT = (data: JoypadData) => void;

const InitialJPvalue: JoypadData = {
  axis: [0, 0, 0, 0],
  buttons: [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  name: "",
  connected: false
}

export const useJoypad = () => {
  const [joypad, setJoypad] = useState<JoypadData>(InitialJPvalue);

  let onUpdateHandler: JoypadUpdateHandlerT = (data) => {
    // console.log("default joypad update handler : JoypadData = ", data)
  };
  const onUpdate = (f: JoypadUpdateHandlerT) => {
    onUpdateHandler = f;
  }

  const connecthandler = (e: any) => {
    const gamepad = e?.gamepad || {};
    console.log("Gamepad Connected!!")
    const name = gamepad?.id || "unknown";
    window.addEventListener('joystickmove', (_: any) => update());
    // window.addEventListener('buttonpress', (e: any) => showPressedButton(e.index));
    // window.addEventListener('buttonrelease', (e: any) => removePressedButton(e.index));
    setJoypad({...joypad, name,connected:true });
  }   

  const disconnecthandler = (e: any) => {
    console.log("Diconnected" + e.gamepad.index);
    setJoypad({...joypad, connected:false});
  }   

  const update = () => {
    // let pads = navigator.getGamepads ? navigator.getGamepads() : (navigator.webkitGetGamepads ? navigator.webkitGetGamepads : []);
    const pads = navigator.getGamepads();
    const pad = pads[0];
    if (!pad) return;
    let but = [];
    for (let i = 0; i < pad.buttons.length; i++) {
      let val = pad.buttons[i];
      but[i] = (typeof (val) === "object") ? val.value : val;
    }
    let new_jp = InitialJPvalue;
    new_jp.connected = true;
    new_jp.name = joypad.name;
    new_jp.axis = pad.axes.concat();
    new_jp.buttons = but.concat();
    if (new_jp.axis.length > 4) {
      const L = 11;
      new_jp.buttons[L] = pad.axes[4] === 1 ? 1 : 0;
      new_jp.buttons[L + 1] = pad.axes[4] === -1 ? 1 : 0;
      new_jp.buttons[L + 2] = pad.axes[5] === 1 ? 1 : 0;
      new_jp.buttons[L + 3] = pad.axes[5] === -1 ? 1 : 0;
      new_jp.axis = [
        pad.axes[0],
        pad.axes[1],
        pad.axes[2],
        pad.axes[3],
      ]
    }
    new_jp.buttons.length = 16;
    for (let i = 0; i < new_jp.axis.length; i++) new_jp.axis[i] = Math.floor(new_jp.axis[i] * 100);
    setJoypad({...new_jp});
    onUpdateHandler(new_jp);
  }

  useEffect(() => {
    window.addEventListener("gamepadconnected", connecthandler);
    window.addEventListener("gamepaddisconnected", disconnecthandler);
  }, []);

  useEffect(() => {
    const id = setInterval(update,50)
    return () => clearInterval(id) 
  }, []);

  return [joypad, { onUpdate }] as const;
}

