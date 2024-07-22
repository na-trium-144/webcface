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
        return wcfBadClient;
    }
    wcli_list.erase(std::find(wcli_list.begin(), wcli_list.end(), wcli));
    delete wcli_;
    return wcfOk;
}

wcfStatus wcfStart(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->start();
    return wcfOk;
}
wcfStatus wcfWaitConnection(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->waitConnection();
    return wcfOk;
}
wcfStatus wcfAutoReconnect(wcfClient *wcli, int enabled) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->autoReconnect(enabled);
    return wcfOk;
}
wcfStatus wcfRecv(wcfClient *wcli, int timeout) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->recv(std::chrono::microseconds(timeout));
    return wcfOk;
}
wcfStatus wcfWaitRecv(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->waitRecv();
    return wcfOk;
}
wcfStatus wcfAutoRecv(wcfClient *wcli, int interval) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->autoRecv(interval >= 1, std::chrono::microseconds(interval));
    return wcfOk;
}
wcfStatus wcfSync(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->sync();
    return wcfOk;
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
            return wcfOk;
        }
    }
    {
        auto fw_ptr = static_cast<const wcfMultiValW *>(ptr);
        auto fw_it = func_val_list_w.find(fw_ptr);
        if (fw_it != func_val_list_w.end()) {
            func_val_list_w.erase(fw_it);
            delete fw_ptr;
            return wcfOk;
        }
    }
    {
        auto v_ptr = static_cast<const wcfViewComponent *>(ptr);
        auto v_it = view_list.find(v_ptr);
        if (v_it != view_list.end()) {
            view_list.erase(v_it);
            delete[] v_ptr;
            return wcfOk;
        }
    }
    {
        auto vw_ptr = static_cast<const wcfViewComponentW *>(ptr);
        auto vw_it = view_list_w.find(vw_ptr);
        if (vw_it != view_list_w.end()) {
            view_list_w.erase(vw_it);
            delete[] vw_ptr;
            return wcfOk;
        }
    }
    return wcfBadHandle;
}
}

/// \private
template <typename CharT>
static wcfStatus wcfMemberListT(wcfClient *wcli, const CharT **list, int size,
                                int *members_num) {
    *members_num = 0;
    if (size < 0) {
        return wcfInvalidArgument;
    }
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    const auto &members = wcli_->members();
    *members_num = static_cast<int>(members.size());
    for (std::size_t i = 0;
         i < static_cast<std::size_t>(size) && i < members.size(); i++) {
        if constexpr (std::is_same_v<CharT, char>) {
            list[i] = members.at(i).name().c_str();
        } else {
            list[i] = members.at(i).nameW().c_str();
        }
    }
    return wcfOk;
}
/// \private
template <typename CharT>
static wcfStatus
wcfMemberEntryEventT(wcfClient *wcli,
                     typename CharType<CharT>::CEventCallback1 callback,
                     void *user_data) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->onMemberEntry([callback, user_data](const Member &m) {
        if constexpr (std::is_same_v<CharT, char>) {
            callback(m.name().c_str(), user_data);
        } else {
            callback(m.nameW().c_str(), user_data);
        }
    });
    return wcfOk;
}

extern "C" {
wcfStatus wcfMemberList(wcfClient *wcli, const char **list, int size,
                        int *members_num) {
    return wcfMemberListT(wcli, list, size, members_num);
}
wcfStatus wcfMemberListW(wcfClient *wcli, const wchar_t **list, int size,
                         int *members_num) {
    return wcfMemberListT(wcli, list, size, members_num);
}
wcfStatus wcfMemberEntryEvent(wcfClient *wcli, wcfEventCallback1 callback,
                              void *user_data) {
    return wcfMemberEntryEventT<char>(wcli, callback, user_data);
}
wcfStatus wcfMemberEntryEventW(wcfClient *wcli, wcfEventCallback1W callback,
                               void *user_data) {
    return wcfMemberEntryEventT<wchar_t>(wcli, callback, user_data);
}

const char *wcfServerVersion(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return "";
    }
    return wcli_->serverVersion().c_str();
}
const char *wcfServerName(wcfClient *wcli) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return "";
    }
    return wcli_->serverName().c_str();
}
}
