export type TerminalLogT = {
  message: string;
  timestamp: Date;
  level: number;
};

export type AnyValue = number | boolean | string;
export type FromRobotDataT = {
  name: string;
  data: AnyValue[];
  lastdata: AnyValue;
  timestamp: Date;
};

// export type FromRobotSettingT = ArgInfo;
export type FromRobotSettingT = {
  name: string;
  type: string;
  isSelected?: boolean; // 送られるデータには含まれないがlog.tsxで使
};
export interface FromRobotSettingGroupT {
  name: string;
  isOpened?: boolean;
}
export interface FromRobotSettingServerT {
  name: string;
  isOpened?: boolean;
  elements: (FromRobotSettingT | FromRobotSettingGroupT)[];
}
export type ImageDataT = {
  name: string;
  src: string;
};
export type ArgInfo = {
  name: string;
  type: string;
};
export type ToRobotSettingT = {
  name: string;
  value: ArgInfo[];
};
export type FunctionSettingT = {
  name: string;
  args: ArgInfo[];
};
export type ImageSettingT = {
  name: string;
};

export type CustomPageComponentT = {
  type: string;
  value?: CustomPageValueT;
  children?: CustomPageComponentT[];
  display_name?: CustomPageValueT;
  callback?: CustomPageCallbackT;
  color?: CustomPageValueT;
  layers?: string[];
};
export type CustomPageLayoutSettingT = {
  name: string;
  layout: CustomPageComponentT;
};
export type CustomPageValueT = {
  value?: AnyValue;
  value_name?: string;
};
export type CustomPageCallbackT = {
  callback_name: string;
  args?: CustomPageValueT[];
};
export type DrawingLayerT = {
  name: string;
  layer: CustomPageComponentT[];
};
export type RelatedServerSettingT = {
  addr: string;
  port: number;
};
export type ErrorInfoT = {
  func: string;
  args: AnyValue[];
  message: string;
};

export interface SocketContextI {
  isConnected: boolean;
  runCallback: (name: string, args: AnyValue) => boolean;
  setToRobot: (name: string, value: AnyValue) => boolean;
  socketUrl: string;
  serverName: string;
  hostname: string;
  version: string;
  getDataAmount: () => number;

  // setting
  functionSetting: FunctionSettingT[];
  toRobotSetting: ToRobotSettingT[];
  fromRobotSetting: FromRobotSettingT[];
  // customPageLayoutSetting: CustomPageLayoutSettingT[];
  imageSetting: ImageSettingT[];
  relatedServerSetting: RelatedServerSettingT[];
  gamepadButtonSetting: string[];
  gamepadAxisSetting: string[];
  gamepadButtonMap: number[];
  gamepadAxisMap: number[];

  getCustomPageLayout: () => CustomPageLayoutSettingT[];
  getDrawingLayer: () => DrawingLayerT[];

  // fromRobot
  // Contextに値を直接入れると、値が更新されるたびに再レンダリングが発生してしまう
  // のでFromRobotとTerminalLogに関してはget関数を用意する
  getFromRobotData: () => FromRobotDataT[];
  findFromRobotData: (name: string) => FromRobotDataT;
  clearFromRobotValue: (name: string) => void;

  //terminalLog
  getTerminalLog: () => TerminalLogT[];

  //image
  getImageData: () => ImageDataT[];

  //error
  getErrorMessage: () => ErrorInfoT[];

  //gamepad
  gamepadConnectIndex: number | null;
  setGamepadConnectIndex: (index: number | null) => void;

  getAlert: () => string;
  clearAlert: () => void;
}

export interface MultiSocketContextI {
  raw: SocketContextI[];
  get: (serverName: string) => SocketContextI;
  // set: (serverName: string, value: SocketContextI) => void;
  getByIndex: (index: number) => SocketContextI;
  setByIndex: (index: number, value: SocketContextI) => void;
  add: () => void;
  // getNames: () => string[];
}

export const parseName = (name: string) => {
  // 他サーバーにある関数や変数の名前をパース
  const serverName = name.slice(0, name.indexOf(":"));
  const funcName = name.slice(name.indexOf(":") + 1);
  return [serverName, funcName];
};
