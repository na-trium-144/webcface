import { useEffect, useState, useRef } from "react";
import { Card, CardContent, Grid, Typography, Divider } from "@mui/material";
import { useSocket } from "../components/socketContext";
import Image from "next/image";
import { ImageDataT } from "lib/global";

export default function CameraView() {
  const socket = useSocket();
  const [timestamp, setTimestamp] = useState("");
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
        {imageData.map((im, si) => im.map((img) => (
          <Grid item xs={12} md={6} lg={4} xl={3} key={`${socket.getByIndex(si).serverName}:${img.name}`}>
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
                  <Image
                    src={img.src}
                    alt={`${socket.getByIndex(si).serverName}:${img.name}`}
                    fill
                    style={{ objectFit: "contain" }}
                  />
                </div>
              </CardContent>
            </Card>
          </Grid>
        )))}
      </Grid>
    </>
  );
}
