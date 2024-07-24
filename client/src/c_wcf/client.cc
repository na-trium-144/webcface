#include "c_wcf_internal.h"

extern "C" {
wcfClient *wcfInit(const char *name, const char *host, int port) {
    auto wcli = new Client(strOrEmpty(name), host ? host : "127.0.0.1", port);
    wcli_list.push_back(wcli);
    return wcli;
}
wcfClient *wcfInitW(const wchar_t *name, const wchar_t *host, int port) {
    auto wcli = new Client(strOrEmpty(name), host ? host : L"127.0.0.1", port);
    wcli_list.push_back(wcli);
    return wcli;
}
wcfClient *wcfInitDefault(const char *name) {
    return wcfInit(name, "127.0.0.1", WEBCFACE_DEFAULT_PORT);
}
wcfClient *wcfInitDefaultW(const wchar_t *name) {
    return wcfInitW(name, L"127.0.0.1", WEBCFACE_DEFAULT_PORT);
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
wcfStatus wcfWaitConnection(wcfClient *wcli, int interval) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->waitConnection(std::chrono::microseconds(interval));
    return WCF_OK;
}
wcfStatus wcfAutoReconnect(wcfClient *wcli, int enabled) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->autoReconnect(enabled);
    return WCF_OK;
}
wcfStatus wcfRecv(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->recv();
    return WCF_OK;
}
wcfStatus wcfWaitRecvFor(wcfClient *wcli, int timeout) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->waitRecvFor(std::chrono::microseconds(timeout));
    return WCF_OK;
}
wcfStatus wcfWaitRecv(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->waitRecv();
    return WCF_OK;
}
wcfStatus wcfAutoRecv(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return WCF_BAD_WCLI;
    }
    wcli_->autoRecv(true);
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
    wcfMultiVal val;
    val.as_int = value;
    val.as_double = 0;
    val.as_str = 0;
    return val;
}
wcfMultiVal wcfValD(double value) {
    wcfMultiVal val;
    val.as_int = 0;
    val.as_double = value;
    val.as_str = 0;
    return val;
}
wcfMultiVal wcfValS(const char *value) {
    wcfMultiVal val;
    val.as_int = 0;
    val.as_double = 0;
    val.as_str = value;
    return val;
}

wcfStatus wcfDestroy(const void *ptr) {
    {
        auto f_ptr = static_cast<const wcfMultiVal *>(ptr);
        auto f_it = func_val_list.find(f_ptr);
        if (f_it != func_val_list.end()) {
            func_val_list.erase(f_it);
            delete f_ptr;
            return WCF_OK;
        }
    }
    {
        auto fw_ptr = static_cast<const wcfMultiValW *>(ptr);
        auto fw_it = func_val_list_w.find(fw_ptr);
        if (fw_it != func_val_list_w.end()) {
            func_val_list_w.erase(fw_it);
            delete fw_ptr;
            return WCF_OK;
        }
    }
    {
        auto v_ptr = static_cast<const wcfViewComponent *>(ptr);
        auto v_it = view_list.find(v_ptr);
        if (v_it != view_list.end()) {
            view_list.erase(v_it);
            delete[] v_ptr;
            return WCF_OK;
        }
    }
    {
        auto vw_ptr = static_cast<const wcfViewComponentW *>(ptr);
        auto vw_it = view_list_w.find(vw_ptr);
        if (vw_it != view_list_w.end()) {
            view_list_w.erase(vw_it);
            delete[] vw_ptr;
            return WCF_OK;
        }
    }
    return WCF_BAD_HANDLE;
}
}
