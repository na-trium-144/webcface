import TransportStream from "winston-transport";

export interface LogLine {
  level: number;
  time: Date;
  message: string;
}

function levelWinstonToSys(level: string) {
  switch (level) {
    case "emerg":
      return 0;
    case "alert":
      return 1;
    case "crit":
      return 2;
    case "error":
      return 3;
    case "warn":
      return 4;
    case "notice":
      return 5;
    case "info":
    case "http":
      return 6;
    case "verbose":
    case "debug":
      return 7;
    case "silly":
      return 8;
    default:
      return 5;
  }
}
export class LogQueueTransport extends TransportStream {
  logQueue: LogLine[];
  constructor(opts: TransportStream.TransportStreamOptions) {
    super(opts);
    this.logQueue = [];
  }
  log(info: { level: string; message: string }, callback: () => void) {
    const levelNum = levelWinstonToSys(info.level);
    this.logQueue.push({
      level: levelNum,
      message: info.message,
      time: new Date(),
    });
    // Perform the writing to the remote service
    callback();
  }
}
