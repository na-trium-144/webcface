# Interface for C
![c++ ver1.3](https://img.shields.io/badge/1.3~-00599c?logo=C%2B%2B)

API Reference → webcface/c_wcf.h

CからアクセスできるAPIとして、wcf〜 で始まる名前の関数やstructが用意されています。
`<webcface/c_wcf.h>` をincludeすることで使えます。

ほとんどの関数は戻り値がint型で、成功した場合0(=`WCF_OK`)、例外が発生した場合正の値を返します。

## Client

(詳細な説明はC++用の[Client](01_client.md)の章も参照)

```c
wcfClient *wcli = wcfInitDefault("sample");
wcfStart(wcli);
```
でclientを生成し、接続します。
サーバーのアドレスとポートを指定したい場合`wcfInit()`を使います

```c
while(1){
    // ...
    wcfSync(wcli);
}
```
のように `wcfSync` をすることで実際にデータを送信します。

```c
wcfClose(wcli);
```
で接続を切り、wcliオブジェクトを破棄します。

## Value

(詳細な説明はC++用の[Value](10_value.md)の章も参照)

### 送信

double型の単一の値は
```c
wcfValueSet(wcli, "hoge", 123.45);
```

配列データは
```c
double value[5] = {1, 2, 3, 4, 5};
wcfValueSetVecD(wcli, "fuga", value, 5);
```
のように送信できます。

### 受信

```c
double value[5];
int size;
int ret = wcfValueGetVecD(wcli, "a", "hoge", value, 5, &size);
// ex.) ret = WCF_OK, value = {123.45, 0, 0, 0, 0}, size = 1
```
sizeに受信した値の個数、valueに受信した値が入ります。

初回の呼び出しでは`WCF_NOT_FOUND`を返し、別スレッドでリクエストが送信されます。

## Func

(詳細な説明はC++用の[Func](30_func.md)の章も参照)

### 関数の呼び出し

```c
wcfMultiVal args[3] = {
    wcfValI(42),
    wcfValD(1.5),
    wcfValS("aaa"),
};
wcfMultiVal *ret;
wcfFuncRun(wcli_, "a", "b", args, 3, &ret);
// ex.) ret->as_double = 123.45
```
関数が存在しない場合`WCF_NOT_FOUND`を返します。
関数が例外を投げた場合`WCF_EXCEPTION`を返し、ret->as_strにエラーメッセージが入ります。

### 関数の待ち受け (他clientから呼び出される)

```c
wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
wcfFuncListen(wcli, "hoge", args_type, 3, WCF_VAL_INT);
```
で待ち受けを開始し、func.set()と同様関数が登録され他クライアントから見られるようになります。
(wcfFuncListen()ではブロックはしません)

その後、任意のタイミングで
```c
wcfFuncCallHandle *handle;
wcfFuncFetchCall(wcli, "hoge", &handle);
```
とすることで関数が呼び出されたかどうかを調べることができます。
listen時に指定した引数の個数と呼び出し時の個数が一致しない場合、fetchCallで取得する前に呼び出し元に例外が投げられます(呼び出されていないのと同じことになります)

その関数がまだ呼び出されていない場合は`WCF_NOT_CALLED`が返ります。
関数が呼び出された場合`WCF_OK`が返り、`handle->args`に引数が格納されます。
```c
wcfMultiVal res = wcfValD(123.45);
wcfFuncRespond(handle, &res);
```
で関数のreturnと同様に関数を終了して値を返したり、
```c
wcfFuncReject(handle, "エラーメッセージ");
```
でエラーメッセージを返すことができます(呼び出し元にはruntime_errorを投げたものとして返ります)

