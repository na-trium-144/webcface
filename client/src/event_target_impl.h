#pragma once
#include <webcface/common/def.h>
#include <webcface/event_target.h>

WEBCFACE_NS_BEGIN

// MinGWのバグにより、メンバ関数を別で定義しないとexportされない

template <typename ArgType>
EventTarget<ArgType>::~EventTarget() {}

template <typename ArgType>
typename EventTarget<ArgType>::CallbackList &
EventTarget<ArgType>::callbackList() const {
    onAppend();
    return checkCl();
}

template <typename ArgType>
typename EventTarget<ArgType>::EventHandle EventTarget<ArgType>::appendListener(
    const typename EventTarget<ArgType>::EventCallback &callback) const {
    onAppend();
    return checkCl().append(callback);
}

template <typename ArgType>
typename EventTarget<ArgType>::EventHandle
EventTarget<ArgType>::prependListener(
    const typename EventTarget<ArgType>::EventCallback &callback) const {
    onAppend();
    return checkCl().prepend(callback);
}
template <typename ArgType>
typename EventTarget<ArgType>::EventHandle EventTarget<ArgType>::insertListener(
    const typename EventTarget<ArgType>::EventCallback &callback,
    const EventHandle &before) const {
    onAppend();
    return checkCl().insert(callback, before);
}

template <typename ArgType>
bool EventTarget<ArgType>::removeListener(
    const typename EventTarget<ArgType>::EventHandle &handle) const {
    return checkCl().remove(handle);
}

template <typename ArgType>
bool EventTarget<ArgType>::hasAnyListener() const {
    return !checkCl().empty();
}
template <typename ArgType>
bool EventTarget<ArgType>::ownsHandle(
    const typename EventTarget<ArgType>::EventHandle &handle) const {
    return checkCl().ownsHandle(handle);
}

WEBCFACE_NS_END
