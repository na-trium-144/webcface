#include "webcface/common/c_val_adaptor.h"

extern "C" {
wcfMultiVal wcfValI(int value) {
    wcfMultiVal val;
    val.as_int = value;
    val.as_double = 0;
    val.as_str = nullptr;
    return val;
}
wcfMultiVal wcfValD(double value) {
    wcfMultiVal val;
    val.as_int = 0;
    val.as_double = value;
    val.as_str = nullptr;
    return val;
}
wcfMultiVal wcfValS(const char *value) {
    wcfMultiVal val;
    val.as_int = 0;
    val.as_double = 0;
    val.as_str = value;
    return val;
}
wcfMultiValW wcfValWI(int value) {
    wcfMultiValW val;
    val.as_int = value;
    val.as_double = 0;
    val.as_str = nullptr;
    return val;
}
wcfMultiValW wcfValWD(double value) {
    wcfMultiValW val;
    val.as_int = 0;
    val.as_double = value;
    val.as_str = nullptr;
    return val;
}
wcfMultiValW wcfValWS(const wchar_t *value) {
    wcfMultiValW val;
    val.as_int = 0;
    val.as_double = 0;
    val.as_str = value;
    return val;
}
}
