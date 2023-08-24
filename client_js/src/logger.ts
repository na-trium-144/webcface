
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
  isEqualTo(otherLevel: log4jsLevel): boolean;
  isLessThanOrEqualTo(otherLevel: log4jsLevel): boolean;
  isGreaterThanOrEqualTo(otherLevel: log4jsLevel): boolean;
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

export function log4jsLevelConvert(level: log4jsLevel, levels:log4jsLevels){
  if(level.isGreaterThanOrEqualTo(levels.FATAL)){
    return 5;
  }else if(level.isGreaterThanOrEqualTo(levels.ERROR)){
    return 4;
  }else if(level.isGreaterThanOrEqualTo(levels.WARN)){
    return 3;
  }else if(level.isGreaterThanOrEqualTo(levels.INFO)){
    return 2;
  }else if(level.isGreaterThanOrEqualTo(levels.DEBUG)){
    return 1;
  }else if(level.isGreaterThanOrEqualTo(levels.TRACE)){
    return 0;
  }else{
    return -1;
  }
}
