
export interface LogLine {
  level: number;
  time: Date;
  message: string;
}

export interface log4jsLevels {
  ALL: log4jsLevel;
  MARK: log4jsLevel;
  TRACE: log4jsLevel;
  DEBUG: log4jsLevel;
  INFO: log4jsLevel;
  WARN: log4jsLevel;
  ERROR: log4jsLevel;
  FATAL: log4jsLevel;
  OFF: log4jsLevel;
  levels: log4jsLevel[];
  getLevel(log4jsLevel: log4jsLevel | string, defaultLevel?: log4jsLevel): log4jsLevel;
  addLevels(customLevels: object): void;
}

export interface log4jsLevel {
  colour: string;
  level: number;
  levelStr: string;
}
export interface log4jsLoggingEvent {
  categoryName: string; // name of category
  level: log4jsLevel; // level of message
  data: any[]; // objects to log
  startTime: Date;
}

function levelLog4ToSys(level: string) {
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
