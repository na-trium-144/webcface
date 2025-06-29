#include "webcface/exception.h"
#include "webcface/field.h"

WEBCFACE_NS_BEGIN

FuncNotFound::FuncNotFound(const FieldBase &base)
    : std::runtime_error("Func(\"" + base.field_.decode() +
                         "\") does not exist in member(\"" +
                         base.member_.decode() +
                         "\"), "
                         "or client not connected to server") {}

SanityError::SanityError(const char *message) noexcept
    : std::runtime_error(message) {}

Intrusion::Intrusion(const FieldBase &base)
    : std::invalid_argument("Cannot modify data of member(\"" +
                            base.member_.decode() + "\")") {}

WEBCFACE_NS_END
