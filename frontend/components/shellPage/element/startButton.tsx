import PlayCircleIcon from '@mui/icons-material/PlayCircle';
import {IconButton} from '@mui/material';

export default function StartButton(props: {onClick: ()=>void}) {
  return (
    <IconButton edge='start' onClick={props.onClick}>
      <PlayCircleIcon fontSize="large" />
    </IconButton>
  );
}
