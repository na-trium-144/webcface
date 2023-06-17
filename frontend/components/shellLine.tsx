import BookmarkBorderIcon from "@mui/icons-material/BookmarkBorder";
import BookmarkIcon from "@mui/icons-material/Bookmark";
import PlayCircleIcon from "@mui/icons-material/PlayCircle";
import {
  Checkbox,
  IconButton,
  Stack,
  TextField,
  Typography,
} from "@mui/material";

type Argument = {
  name: string;
  defaultValue: any;
};

type Props = {
  name: string;
  arguments: Array<Argument>;
  onClick: Function;
};

export default function ShellLine(props: Props) {
  return (
    <Stack direction="row" spacing={2} alignItems="center">
      <Checkbox icon={<BookmarkBorderIcon />} checkedIcon={<BookmarkIcon />} />

      <Typography variant="h4">{props.name}</Typography>

      {props.arguments.map((arg: Argument) => {
        return (
          <Stack direction="row" spacing={0} alignItems="center" key={arg.name}>
            <Typography variant="h5">{`${arg.name}= `}</Typography>
            <TextField
              required
              label="required"
              variant="filled"
              type={typeof arg.defaultValue}
              size="small"
              defaultValue={arg.defaultValue}
            />
          </Stack>
        );
      })}

      <IconButton onClick={() => props.onClick}>
        <PlayCircleIcon fontSize="large" />
      </IconButton>
    </Stack>
  );
}
