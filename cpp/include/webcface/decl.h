#pragma once
// クラスが相互依存しすぎているのでクラス名の宣言だけ全部ここでする

namespace drogon {
class WebSocketClient;
}
namespace WebCFace {
class Member;
template <typename V>
class MemberEvent;

class Client;

template <typename T>
class SyncData;
template <typename T, typename V>
class SyncDataWithEvent;
template <typename T>
class SyncDataKey;
class Value;
class Text;

template <typename T>
class SyncDataStore;

class FuncNotFound;
class FuncInfo;
class AsyncFuncResult;
class FuncResultStore;
class Func;

class ValAdaptor;

} // namespace WebCFace