#include "c_wcf_internal.h"

extern "C" {
wcfClient *wcfInit(const char *name, const char *host, int port) {
    auto wcli = new Client(name ? name : "", host ? host : "127.0.0.1", port);
    wcli_list.push_back(wcli);
    return wcli;
}
wcfClient *wcfInitDefault(const char *name) {
    return wcfInit(name ? name : "", "127.0.0.1", WEBCFACE_DEFAULT_PORT);
}
int wcfIsValid(wcfClient *wcli) {
    if (getWcli(wcli)) {
        return 1;
    } else {
        return 0;
    }
}
int wcfIsConnected(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (wcli_ && wcli_->connected()) {
        return 1;
    } else {
        return 0;
    }
}
wcfStatus wcfClose(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_list.erase(std::find(wcli_list.begin(), wcli_list.end(), wcli));
    delete wcli_;
    return WCF_OK;
}

wcfStatus wcfStart(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->start();
    return WCF_OK;
}
wcfStatus wcfSync(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->sync();
    return WCF_OK;
}
wcfMultiVal wcfValI(int value) {
    wcfMultiVal val = {.as_int = value, .as_double = 0, .as_str = 0};
    return val;
}
wcfMultiVal wcfValD(double value) {
    wcfMultiVal val = {.as_int = 0, .as_double = value, .as_str = 0};
    return val;
}
wcfMultiVal wcfValS(const char *value) {
    wcfMultiVal val = {.as_int = 0, .as_double = 0, .as_str = value};
    return val;
}

wcfStatus wcfDestroy(const void *ptr) {
    auto f_ptr = static_cast<const wcfMultiVal *>(ptr);
    auto f_it = func_val_list.find(f_ptr);
    if (f_it != func_val_list.end()) {
        func_val_list.erase(f_it);
        delete f_ptr;
        return WCF_OK;
    }
    auto v_ptr = static_cast<const wcfViewComponent *>(ptr);
    auto v_it = view_list.find(v_ptr);
    if (v_it != view_list.end()) {
        view_list.erase(v_it);
        delete[] v_ptr;
        return WCF_OK;
    }
    return WCF_BAD_HANDLE;
}
}
