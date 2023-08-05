#pragma once
// クラスが相互依存しすぎているのでクラス名の宣言だけ全部ここでする

namespace drogon {
class WebSocketClient;
}
namespace WebCFace {
class ValAdaptor;

class EventKey;
template <typename V>
class EventTarget;

class Member;

class Client;

template <typename T>
class SyncData;
class Value;
class Text;

template <typename T>
class SyncDataStore;

class FuncNotFound;
class AsyncFuncResult;
class FuncResultStore;

class Arg;
class FuncInfo;
class Func;

} // namespace WebCFace