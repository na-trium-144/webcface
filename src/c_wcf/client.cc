#include "c_wcf_internal.h"

extern "C" {
wcfClient *wcfInit(const char *name, const char *host, int port) {
    auto wcli = new Client(name, host, port);
    wcli_list.push_back(wcli);
    return wcli;
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
}