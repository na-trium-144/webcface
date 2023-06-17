import { useJoypad } from '../lib/useJoypad'
import JoypadView from '../components/JoypadView'

export default function Admin() {
  const [joypad, { }] = useJoypad();
  return (<>
    <JoypadView joypad={joypad} />
    {JSON.stringify(joypad)}
  </>
  );
}

