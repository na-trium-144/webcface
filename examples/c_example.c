#include <webcface/wcf.h>
#include <stdio.h>

int main(void) {
    wcfClient *wcli = wcfInitDefault("sample");
    wcfStart(wcli);

    int i = 0;
    wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli, "hoge", args_type, 3, WCF_VAL_DOUBLE);

    while (1) {
        wcfValueSet(wcli, "hoge", i++);

        wcfFuncCallHandle *handle;
        if (wcfFuncFetchCall(wcli, "hoge", &handle) == WCF_OK) {
            printf("hoge %d, %f, '%s'\n", handle->args[0].as_int,
                   handle->args[1].as_double, handle->args[2].as_str);
            wcfMultiVal res = wcfValD(123.45);
            wcfFuncRespond(handle, &res);
        }
        // ...
        wcfWaitSyncFor(wcli, 100000);
    }
    return 0;
}
