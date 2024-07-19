#include "c_wcf_internal.h"
#include "webcface/value.h"
#include "webcface/text.h"
#include "webcface/view.h"
#include "webcface/func.h"

template <typename V, typename CharT>
static wcfStatus wcfEntryListT(wcfClient *wcli, const CharT *member,
                               const CharT **list, int size, int *field_num) {
    *field_num = 0;
    if (size < 0) {
        return wcfInvalidArgument;
    }
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    std::vector<V> fields;
    if constexpr (std::is_same_v<V, Value>) {
        fields = wcli_->member(strOrEmpty(member)).valueEntries();
    } else if constexpr (std::is_same_v<V, Text>) {
        fields = wcli_->member(strOrEmpty(member)).textEntries();
    } else if constexpr (std::is_same_v<V, View>) {
        fields = wcli_->member(strOrEmpty(member)).viewEntries();
    } else if constexpr (std::is_same_v<V, Func>) {
        fields = wcli_->member(strOrEmpty(member)).funcEntries();
    }
    *field_num = static_cast<int>(fields.size());
    for (std::size_t i = 0;
         i < static_cast<std::size_t>(size) && i < fields.size(); i++) {
        if constexpr (std::is_same_v<CharT, char>) {
            list[i] = fields.at(i).name().c_str();
        } else {
            list[i] = fields.at(i).nameW().c_str();
        }
    }
    return wcfOk;
}
template <typename V, typename CharT>
static wcfStatus
wcfEntryEventT(wcfClient *wcli, const CharT *member,
               typename CharType<CharT>::CEventCallback2 callback,
               void *user_data) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    auto cb = [callback, user_data](const V &m) {
        if constexpr (std::is_same_v<CharT, char>) {
            callback(m.member().name().c_str(), m.name().c_str(), user_data);
        } else {
            callback(m.member().nameW().c_str(), m.nameW().c_str(), user_data);
        }
    };
    if constexpr (std::is_same_v<V, Value>) {
        wcli_->member(strOrEmpty(member)).onValueEntry(cb);
    } else if constexpr (std::is_same_v<V, Text>) {
        wcli_->member(strOrEmpty(member)).onTextEntry(cb);
    } else if constexpr (std::is_same_v<V, View>) {
        wcli_->member(strOrEmpty(member)).onViewEntry(cb);
    } else if constexpr (std::is_same_v<V, Func>) {
        wcli_->member(strOrEmpty(member)).onFuncEntry(cb);
    }
    return wcfOk;
}
template <typename CharT>
static wcfStatus
wcfSyncEventT(wcfClient *wcli, const CharT *member,
              typename CharType<CharT>::CEventCallback1 callback,
              void *user_data) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return wcfBadClient;
    }
    wcli_->member(strOrEmpty(member))
        .onSync([callback, user_data](const Member &m) {
            if constexpr (std::is_same_v<CharT, char>) {
                callback(m.name().c_str(), user_data);
            } else {
                callback(m.nameW().c_str(), user_data);
            }
        });
    return wcfOk;
}

extern "C" {

wcfStatus wcfValueEntryList(wcfClient *wcli, const char *member,
                            const char **list, int size, int *field_num) {
    return wcfEntryListT<Value>(wcli, member, list, size, field_num);
}
wcfStatus wcfValueEntryListW(wcfClient *wcli, const wchar_t *member,
                             const wchar_t **list, int size, int *field_num) {
    return wcfEntryListT<Value>(wcli, member, list, size, field_num);
}
wcfStatus wcfTextEntryList(wcfClient *wcli, const char *member,
                           const char **list, int size, int *field_num) {
    return wcfEntryListT<Text>(wcli, member, list, size, field_num);
}
wcfStatus wcfTextEntryListW(wcfClient *wcli, const wchar_t *member,
                            const wchar_t **list, int size, int *field_num) {
    return wcfEntryListT<Text>(wcli, member, list, size, field_num);
}
wcfStatus wcfFuncEntryList(wcfClient *wcli, const char *member,
                           const char **list, int size, int *field_num) {
    return wcfEntryListT<Func>(wcli, member, list, size, field_num);
}
wcfStatus wcfFuncEntryListW(wcfClient *wcli, const wchar_t *member,
                            const wchar_t **list, int size, int *field_num) {
    return wcfEntryListT<Func>(wcli, member, list, size, field_num);
}
wcfStatus wcfViewEntryList(wcfClient *wcli, const char *member,
                           const char **list, int size, int *field_num) {
    return wcfEntryListT<View>(wcli, member, list, size, field_num);
}
wcfStatus wcfViewEntryListW(wcfClient *wcli, const wchar_t *member,
                            const wchar_t **list, int size, int *field_num) {
    return wcfEntryListT<View>(wcli, member, list, size, field_num);
}

wcfStatus wcfValueEntryEvent(wcfClient *wcli, const char *member,
                             wcfEventCallback2 callback, void *user_data) {
    return wcfEntryEventT<Value>(wcli, member, callback, user_data);
}
wcfStatus wcfValueEntryEventW(wcfClient *wcli, const wchar_t *member,
                              wcfEventCallback2W callback, void *user_data) {
    return wcfEntryEventT<Value>(wcli, member, callback, user_data);
}
wcfStatus wcfTextEntryEvent(wcfClient *wcli, const char *member,
                            wcfEventCallback2 callback, void *user_data) {
    return wcfEntryEventT<Text>(wcli, member, callback, user_data);
}
wcfStatus wcfTextEntryEventW(wcfClient *wcli, const wchar_t *member,
                             wcfEventCallback2W callback, void *user_data) {
    return wcfEntryEventT<Text>(wcli, member, callback, user_data);
}
wcfStatus wcfFuncEntryEvent(wcfClient *wcli, const char *member,
                            wcfEventCallback2 callback, void *user_data) {
    return wcfEntryEventT<Func>(wcli, member, callback, user_data);
}
wcfStatus wcfFuncEntryEventW(wcfClient *wcli, const wchar_t *member,
                             wcfEventCallback2W callback, void *user_data) {
    return wcfEntryEventT<Func>(wcli, member, callback, user_data);
}
wcfStatus wcfViewEntryEvent(wcfClient *wcli, const char *member,
                            wcfEventCallback2 callback, void *user_data) {
    return wcfEntryEventT<View>(wcli, member, callback, user_data);
}
wcfStatus wcfViewEntryEventW(wcfClient *wcli, const wchar_t *member,
                             wcfEventCallback2W callback, void *user_data) {
    return wcfEntryEventT<View>(wcli, member, callback, user_data);
}

wcfStatus wcfMemberSyncEvent(wcfClient *wcli, const char *member,
                             wcfEventCallback1 callback, void *user_data) {
    return wcfSyncEventT(wcli, member, callback, user_data);
}
wcfStatus wcfMemberSyncEventW(wcfClient *wcli, const wchar_t *member,
                              wcfEventCallback1W callback, void *user_data) {
    return wcfSyncEventT(wcli, member, callback, user_data);
}

unsigned long long wcfMemberSyncTime(wcfClient *wcli, const char *member) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               wcli_->member(strOrEmpty(member)).syncTime().time_since_epoch())
        .count();
}
unsigned long long wcfMemberSyncTimeW(wcfClient *wcli, const wchar_t *member) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return 0;
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               wcli_->member(strOrEmpty(member)).syncTime().time_since_epoch())
        .count();
}
const char *wcfMemberLibName(wcfClient *wcli, const char *member) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return "";
    }
    return wcli_->member(strOrEmpty(member)).libName().c_str();
}
const char *wcfMemberLibVersion(wcfClient *wcli, const char *member) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return "";
    }
    return wcli_->member(strOrEmpty(member)).libVersion().c_str();
}
const char *wcfMemberRemoteAddr(wcfClient *wcli, const char *member) {
    auto wcli_ = getWcli(wcli);
    if (!wcli_) {
        return "";
    }
    return wcli_->member(strOrEmpty(member)).remoteAddr().c_str();
}
}
