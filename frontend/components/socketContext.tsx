import { useContext, createContext, useEffect, useState, useRef } from "react";
import {
  FromRobotSettingT,
  ToRobotSettingT,
  FunctionSettingT,
  TerminalLogT,
  FromRobotDataT,
  ImageSettingT,
  ImageDataT,
  CustomPageLayoutSettingT,
  CustomPageComponentT,
  RelatedServerSettingT,
  AnyValue,
  SocketContextI,
  MultiSocketContextI,
} from "../lib/global";

const SocketContext = createContext<MultiSocketContextI>();

export const useSocket = () => {
  return useContext(SocketContext);
};

// 接続しているサーバーの情報はSocketProvider内のsocketContextValuesにすべて入っている
// サーバーの区別はすべてsocketContextValuesのindexで管理する
// 一度接続したサーバーのindexが途中で変わることはない
// 新しく接続したサーバーはsocketContextValuesの末尾に追加される
// 接続を切ってサーバーをsocketContextValuesから削除するようなことはない
const SocketImpl = (props: { index: number }) => {
  const { index } = props; // socketContextValuesのindex

  // 一定時間ごとにFlagを切り替え、再接続のuseEffectを発動させる
  const [reconnectFlag, setReconnectFlag] = useState<boolean>(false);
  useEffect(() => {
    const i = setTimeout(
      () => setReconnectFlag((reconnectFlag) => !reconnectFlag),
      5000
    );
    return () => clearTimeout(i);
  }, [reconnectFlag, setReconnectFlag]);

  const socket = useSocket();
  // 最新のsocketContextValues[index]と同じものがsocketContextValueに入る
  // (useRefを経由するのはsocketContextValuesの変化が後のuseEffectに影響しないようにするため)
  // socketUrlが変化したらreconnectFlagを発動させる
  const socketContextValue = useRef<SocketContextI>({});
  const socketGet = useRef<() => SocketContextI>(() => {});
  useEffect(() => {
    const newValue = socket.getByIndex(index);
    if (socketContextValue.current.socketUrl !== newValue.socketUrl) {
      setReconnectFlag((reconnectFlag) => !reconnectFlag);
      socketContextValue.current = newValue;
      if (ws.current && ws.current.close) {
        ws.current.close();
      }
      ws.current = null;
    }
    socketGet.current = socket.get;
  }, [index, socket]);

  const terminalLog = useRef<TerminalLogT[]>([]);
  const fromRobotData = useRef<FromRobotDataT[]>([]);
  const imageData = useRef<ImageDataT[]>([]);
  const errorMessage = useRef<string>("");
  const customPageLayout = useRef<CustomPageLayoutSettingT[]>([]);
  const recvLength = useRef<number>(0);
  const startTime = useRef<Date>(new Date());

  // socketContextValueとsocketContextValues[index]を更新する
  const setSocketContextValue =
    useRef<(oldValue: SocketContextI) => SocketContextI>();
  useEffect(() => {
    setSocketContextValue.current = (
      callback: (oldValue: SocketContextI) => SocketContextI
    ) => {
      socketContextValue.current = callback(socketContextValue.current);
      socket.setByIndex(index, socketContextValue.current);
    };
  }, [socket, index]);

  // これもsocketContextValuesの変化に影響されないようRefに入れる
  const connectRelatedServers = useRef<
    (related_servers: RelatedServerSettingT[]) => void
  >(() => {});
  useEffect(() => {
    connectRelatedServers.current = (
      related_servers: RelatedServerSettingT[]
    ) => {
      for (const rs of related_servers) {
        const url = `${rs.addr}:${rs.port}`;
        if (socket.raw.findIndex((s) => s.socketUrl === url) === -1) {
          socket.add(url);
        }
      }
    };
  }, [socket]);

  // reconnectFlagが変化したとき再接続(すでに接続されてたら何もしない)
  const ws = useRef(null);
  useEffect(() => {
    const reconnect = () => {
      if (socketContextValue.current.isConnected) {
        return;
      }
      if (ws.current != null) {
        console.log("reconnect failed: ws is not null");
        return;
      }
      const socketUrl = socketContextValue.current.socketUrl;
      console.log("trying to connect server : ws://", socketUrl);
      try {
        ws.current = new WebSocket("ws://" + socketUrl);
        const emit = (msgname: string, msg: any) => {
          ws.current.send(JSON.stringify({ msgname, msg }));
        };

        const runCallback = (
          name: string,
          args: AnyValue[] | object | undefined
        ) => {
          if (args == undefined) {
            args = {};
          } else if (Array.isArray(args)) {
            const argNames = socketContextValue.current.functionSetting
              .find((f) => f.name === name)
              .args.map((a) => a.name);
            const argsList = args;
            args = {};
            for (let i = 0; i < argsList.length; i++) {
              args[argNames[i]] = argsList[i];
            }
          }
          const data = {
            name,
            args,
          };
          console.log("call function = ", data);
          emit("function", data);
        };

        const setToRobot = (name: string, value: any) => {
          emit("to_robot", { name, value });
          return true;
        };
        const findFromRobotData = (name: string) => {
          const f = fromRobotData.current.find((m) => m.name === name);
          if (f) {
            return f;
          }
          return {
            name: name,
            data: [],
            lastdata: null,
            timestamp: new Date(0),
          } as FromRobotDataT;
        };

        ws.current.onopen = (_: any) => {
          // 接続成功したとき各種コールバックを設定
          console.log("connected");
          startTime.current = new Date();
          setSocketContextValue.current((oldValue) => ({
            ...oldValue,
            isConnected: true,
            runCallback: runCallback,
            setToRobot: setToRobot,
            getFromRobotData: () => fromRobotData.current,
            findFromRobotData: findFromRobotData,
            getImageData: () => imageData.current,
            getTerminalLog: () => terminalLog.current,
            getErrorMessage: () => errorMessage.current,
            clearErrorMessage: () => {
              errorMessage.current = "";
            },
            getCustomPageLayout: () => customPageLayout.current,
            getDataAmount: () =>
              (recvLength.current /
                (new Date().getTime() - startTime.current.getTime())) *
              1000,
          }));
          terminalLog.current = [];
          const fetchHostname = async () => {
            try {
              const ret = await fetch("http://" + socketUrl + "/_hostinfo");
              const retJson = (await ret.json()) as {
                name: string;
                addr: string;
              };
              setSocketContextValue.current((oldValue) => ({
                ...oldValue,
                hostname: retJson.name,
              }));
            } catch {}
          };
          void fetchHostname();
        };
        ws.current.onclose = (_: any) => {
          // 接続が切れたとき
          setSocketContextValue.current((oldValue) => ({
            ...oldValue,
            isConnected: false,
            runCallback: () => {
              console.error("runCallback failed since socket disconnected");
            },
            setToRobot: () => {
              console.error("setToRobot failed since socket disconnected");
            },
          }));
          ws.current = null;
        };
        ws.current.onmessage = (event: any) => {
          // try {
          const { msgname, msg } = JSON.parse(event.data);
          // console.log("get data = ", JSON.parse(event.data));
          recvLength.current += event.data.length;
          onMessage(msgname as string, msg);
          // } catch (err: any) {
          //   console.error("undefined message type : ", data)
          // }
        };
        ws.current.onerror = (error: string) => {
          console.error(error.message);
        };

        return;
      } catch {
        console.error(`failed to connect ${socketUrl}`);
        if (ws.current && ws.current.close) {
          ws.current.close();
        }
        ws.current = null;
        return;
      }
    };

    const onMessage = (msgname: string, data: any) => {
      switch (msgname) {
        case "setting": {
          setSocketContextValue.current((oldValue) => ({
            ...oldValue,
            functionSetting: data.functions.sort((a, b) =>
              a.name > b.name ? 1 : -1
            ),
            fromRobotSetting: data.from_robot.sort((a, b) =>
              a.name > b.name ? 1 : -1
            ),
            toRobotSetting: data.to_robot.sort((a, b) =>
              a.name > b.name ? 1 : -1
            ),
            imageSetting: data.images.sort((a, b) =>
              a.name > b.name ? 1 : -1
            ),
            relatedServerSetting: data.related_servers,
            serverName: data.server_name,
            version: data.version,
          }));
          connectRelatedServers.current(data.related_servers);
          break;
        }

        case "from_robot": {
          const timestamp = new Date(data.timestamp);
          let notUpdated = fromRobotData.current.map((el) => el.name);
          for (const [name, value] of Object.entries(data)) {
            if (name == "timestamp") {
              continue;
            }
            let idx = fromRobotData.current.findIndex((el) => el.name === name);
            if (idx === -1) {
              idx = fromRobotData.current.length;
              fromRobotData.current.push({
                name: name,
                data: [value],
                lastdata: value,
                timestamp: timestamp,
              });
            } else {
              fromRobotData.current[idx].data = fromRobotData.current[idx].data
                .slice(-1000)
                .concat([value]);
              fromRobotData.current[idx].lastdata = value;
              fromRobotData.current[idx].timestamp = timestamp;
            }
            notUpdated = notUpdated.filter((el) => el !== name);
          }
          // 送られてこなかったデータは、前と同じデータが送られてきたとみなす
          for (const name of notUpdated) {
            const idx = fromRobotData.current.findIndex(
              (el) => el.name === name
            );
            fromRobotData.current[idx].data = fromRobotData.current[idx].data
              .slice(-1000)
              .concat([fromRobotData.current[idx].lastdata]);
            fromRobotData.current[idx].timestamp = timestamp;
          }
          break;
        }

        case "log": {
          for (const m of data) {
            terminalLog.current.unshift({
              timestamp: new Date(m.timestamp),
              message: m.text,
              level: m.level,
            });
          }
          break;
        }
        case "images": {
          for (const [name, value] of Object.entries(data)) {
            let idx = imageData.current.findIndex((el) => el.name === name);
            if (idx === -1) {
              idx = imageData.current.length;
              imageData.current.push({
                name: name,
                src: value,
              });
            } else {
              imageData.current[idx].src = value;
            }
          }
          break;
        }
        case "layout": {
          for (const l of data) {
            let idx = customPageLayout.current.findIndex(
              (el) => el.name === l.name
            );
            if (idx === -1) {
              customPageLayout.current.push({name: l.name, layout:[]});
              idx = customPageLayout.current.length - 1;
            }
            for(const [ci, c] of Object.entries(l.layout)){
              while(customPageLayout.current[idx].layout.length <= parseInt(ci)){
                customPageLayout.current[idx].layout.push(null);
              }
              customPageLayout.current[idx].layout[parseInt(ci)] = c;
            }
          }
          break;
        }
        case "error": {
          // setErrorMessage(data);
          errorMessage.current = data;
          break;
        }
        default: {
          console.error("Invalid messsage received! => ", msgname, data);
        }
      }
    };

    reconnect();
  }, [reconnectFlag, index]);
  return <></>;
};

const socketContextDefaultValue: SocketContextI = {
  isConnected: false,
  runCallback: () => console.error("runCallback failed: not connected"),
  setToRobot: () => console.error("setToRobot failed: not connected"),
  socketUrl: "",
  serverName: "",
  hostname: "",
  version: "",
  functionSetting: [],
  toRobotSetting: [],
  fromRobotSetting: [],
  getCustomPageLayout: () => [],
  imageSetting: [],
  relatedServerSetting: [],
  getFromRobotData: () => [],
  clearFromRobotValue: () => {},
  getTerminalLog: () => [],
  getImageData: () => [],
  getErrorMessage: () => "",
  clearErrorMessage: () => {},
  getDataAmount: () => 0,
};

export const SocketProvider = (props: any) => {
  const [socketContextValues, setSocketContextValues] = useState<
    SocketContextI[]
  >([
    {
      ...socketContextDefaultValue,
    },
  ]);
  const getSocketContextValue = (serverName: string) => {
    const v = socketContextValues.find((c) => c.serverName === serverName);
    if (v) {
      return v;
    } else {
      return socketContextDefaultValue;
    }
  };
  const getSocketContextValueByIndex = (index: number) => {
    if (index < socketContextValues.length) {
      return socketContextValues[index];
    } else {
      return socketContextDefaultValue;
    }
  };
  const setSocketContextValueByIndex = (
    index: number,
    newValue: SocketContextI
  ) =>
    setSocketContextValues((socketContextValues) =>
      socketContextValues.map((oldValue, j) =>
        j === index ? newValue : oldValue
      )
    );
  const addSocketContext = (socketUrl: string) =>
    setSocketContextValues((socketContextValues) =>
      socketContextValues.concat([
        { ...socketContextDefaultValue, socketUrl: socketUrl },
      ])
    );
  useEffect(() => {
    if (getSocketContextValueByIndex(0).socketUrl === "") {
      setSocketContextValueByIndex(0, {
        ...getSocketContextValueByIndex(0),
        socketUrl: `${location.hostname}:${location.port || 80}`,
      });
    }
  }, [socketContextValues]);
  return (
    <SocketContext.Provider
      value={{
        raw: socketContextValues,
        get: getSocketContextValue,
        getByIndex: getSocketContextValueByIndex,
        setByIndex: setSocketContextValueByIndex,
        add: addSocketContext,
      }}
    >
      {props.children}
      {socketContextValues.map((s, i) => (
        <SocketImpl key={i} index={i} />
      ))}{" "}
    </SocketContext.Provider>
  );
};
