import { useEffect, useState, useRef } from "react";
import { Card, CardContent, Grid, Typography, Divider } from "@mui/material";
import { useSocket } from "../components/socketContext";
import Image from "next/image";
import { ImageDataT } from "lib/global";

export const CameraImage = (props: { sid: number; iid: number }) => {
  const socket = useSocket();

  const [imageData, setImageData] = useState<ImageDataT>([]);
  const [updateFlag, setUpdateFlag] = useState(false);
  useEffect(() => {
    setImageData(socket.getByIndex(props.sid).getImageData()[props.iid]);
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 100);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag, socket, props]);
  return (
    <Image
      src={imageData.src}
      alt={`${socket.getByIndex(props.sid).serverName}:${imageData.name}`}
      fill
      style={{ objectFit: "contain" }}
    />
  );
};
export default function CameraView() {
  const socket = useSocket();
  // const [timestamp, setTimestamp] = useState("");
  const [imageData, setImageData] = useState<ImageDataT[][]>([]);
  const [updateFlag, setUpdateFlag] = useState(false);
  useEffect(() => {
    setImageData(socket.raw.map((s) => s.getImageData()));
    const i = setTimeout(() => {
      setUpdateFlag(!updateFlag);
    }, 100);
    return () => {
      clearTimeout(i);
    };
  }, [updateFlag, socket]);

  return (
    <>
      <Grid container spacing={1}>
        {imageData.map((im, si) =>
          im.map((img, ii) => (
            <Grid
              item
              xs={12}
              md={6}
              lg={4}
              xl={3}
              key={`${socket.getByIndex(si).serverName}:${img.name}`}
            >
              <Card variant="outlined">
                <CardContent>
                  <Typography variant="h6" component="h6" align="center">
                    {socket.getByIndex(si).serverName}:{img.name}
                  </Typography>
                  <div
                    style={{
                      position: "relative",
                      width: "100%",
                      aspectRatio: 4 / 3,
                    }}
                  >
                    <CameraImage sid={si} iid={ii} />
                  </div>
                </CardContent>
              </Card>
            </Grid>
          ))
        )}
      </Grid>
    </>
  );
}
