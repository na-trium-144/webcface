#include <webcface/wcf.h>

int main(void) {
    wcfClient *wcli = wcfInitDefault("");
    int i = 0;
    wcfValType args_type[3] = {WCF_VAL_INT, WCF_VAL_DOUBLE, WCF_VAL_STRING};
    wcfFuncListen(wcli, "hoge", args_type, 3, WCF_VAL_FLOAT);

    wcfSync(wcli);
    return 0;
}
