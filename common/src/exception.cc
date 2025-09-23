#include "webcface/common/exception.h"
#include <string>

WEBCFACE_NS_BEGIN
InvalidArgument::InvalidArgument(const char *message)
    : std::invalid_argument(message) {}
InvalidArgument::InvalidArgument(const std::string &message)
    : std::invalid_argument(message) {}
OutOfRange::OutOfRange(const char *message) : std::out_of_range(message) {}
OutOfRange::OutOfRange(const std::string &message)
    : std::out_of_range(message) {}

ValTypeMismatch::ValTypeMismatch(const char *message)
    : InvalidArgument(message) {}
ValTypeMismatch::ValTypeMismatch(const std::string &message)
    : InvalidArgument(message) {}

WEBCFACE_NS_END