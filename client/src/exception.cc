#include "webcface/exception.h"
#include "webcface/field.h"

WEBCFACE_NS_BEGIN

FuncNotFound::FuncNotFound(const FieldBase &base)
    : std::runtime_error(strJoin<char>("Func(\"", base.field_.decode(),
                                       "\") does not exist in member(\"",
                                       base.member_.decode(),
                                       "\"), "
                                       "or client not connected to server")) {}

Rejection::Rejection(const FieldBase &base, const std::string &message)
    : std::runtime_error(strJoin<char>("member(\"", base.member_.decode(),
                                       "\").func(\"", base.field_.decode(),
                                       "\") rejected: ", message)) {}

SanityError::SanityError(const char *message) : std::runtime_error(message) {}

Intrusion::Intrusion(const FieldBase &base)
    : std::invalid_argument(strJoin<char>("Cannot modify data of member(\"",
                                          base.member_.decode(), "\")")) {}

PromiseError::PromiseError(const char *message) : std::runtime_error(message) {}

FuncSignatureMismatch::FuncSignatureMismatch(const char *message)
    : std::invalid_argument(message) {}
FuncSignatureMismatch::FuncSignatureMismatch(const std::string &message)
    : std::invalid_argument(message) {}
InvalidArgument::InvalidArgument(const char *message)
    : std::invalid_argument(message) {}
InvalidArgument::InvalidArgument(const std::string &message)
    : std::invalid_argument(message) {}
OutOfRange::OutOfRange(const char *message) : std::out_of_range(message) {}
OutOfRange::OutOfRange(const std::string &message)
    : std::out_of_range(message) {}

WEBCFACE_NS_END
