#pragma once
#include "val_adaptor.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief ValAdaptorのVector
 * \since ver2.10
 *
 */
class WEBCFACE_DLL ValAdaptorVector {
    mutable std::vector<ValAdaptor> vec;

  public:
    ValAdaptorVector();
    ValAdaptorVector(const ValAdaptor &val);
    ValAdaptorVector &operator=(const ValAdaptor &val);
    ValAdaptorVector(std::vector<ValAdaptor> init);
    ValAdaptorVector &operator=(std::vector<ValAdaptor> init);

    explicit ValAdaptorVector(StringInitializer str);
    ValAdaptorVector &operator=(StringInitializer str);

    template <typename T,
              typename std::enable_if_t<
                  !std::is_same_v<ValAdaptor, T> &&
                      !std::is_same_v<ValAdaptorVector, T> &&
                      !std::is_convertible_v<StringInitializer, T> &&
                      std::is_constructible_v<ValAdaptor, T>,
                  std::nullptr_t> = nullptr>
    explicit ValAdaptorVector(T &&val)
        : ValAdaptorVector(ValAdaptor(std::forward<T>(val))) {}
    template <typename T,
              typename std::enable_if_t<
                  !std::is_same_v<ValAdaptor, T> &&
                      !std::is_same_v<ValAdaptorVector, T> &&
                      !std::is_convertible_v<StringInitializer, T> &&
                      std::is_constructible_v<ValAdaptor, T>,
                  std::nullptr_t> = nullptr>
    ValAdaptorVector &operator=(T &&val) {
        return *this = ValAdaptor(std::forward<T>(val));
    }

    template <typename R,
              typename std::enable_if_t<
                  std::is_convertible_v<
                      decltype(*std::begin(std::declval<R>())), ValAdaptor> &&
                      std::is_convertible_v<
                          decltype(*std::end(std::declval<R>())), ValAdaptor>,
                  std::nullptr_t> = nullptr>
    ValAdaptorVector(const R &range)
        : vec(std::begin(range), std::end(range)) {}
    template <typename R,
              typename std::enable_if_t<
                  std::is_convertible_v<
                      decltype(*std::begin(std::declval<R>())), ValAdaptor> &&
                      std::is_convertible_v<
                          decltype(*std::end(std::declval<R>())), ValAdaptor>,
                  std::nullptr_t> = nullptr>
    ValAdaptorVector &operator=(const R &range) {
        return *this =
                   std::vector<ValAdaptor>(std::begin(range), std::end(range));
    }
    template <typename T, std::size_t N,
              typename std::enable_if_t<std::is_convertible_v<T, ValAdaptor>,
                                        std::nullptr_t> = nullptr>
    ValAdaptorVector(const T (&range)[N])
        : vec(std::begin(range), std::end(range)) {}
    template <typename T, std::size_t N,
              typename std::enable_if_t<std::is_convertible_v<T, ValAdaptor>,
                                        std::nullptr_t> = nullptr>
    ValAdaptorVector &operator=(const T (&range)[N]) {
        return *this =
                   std::vector<ValAdaptor>(std::begin(range), std::end(range));
    }

    const ValAdaptor &get() const;
    ValType valType() const;

    explicit operator const ValAdaptor&() const { return get(); }
    template <typename T,
              typename std::enable_if_t<std::is_convertible_v<ValAdaptor, T>,
                                        std::nullptr_t> = nullptr>
    operator T() const {
        return get().as<T>();
    }
    template <typename T,
              typename std::enable_if_t<!std::is_convertible_v<ValAdaptor, T>,
                                        std::nullptr_t> = nullptr,
#ifdef _MSC_VER
              // gccでなぜか正しく機能しない
              typename = decltype(std::declval<ValAdaptor>().operator T())>
#else
              // msvcでなぜか正しく機能しない
              typename std::enable_if_t<std::is_constructible_v<T, ValAdaptor>,
                                        std::nullptr_t> = nullptr>
#endif
    explicit operator T() const {
        return get().as<T>();
    }
    template <typename T,
              typename = std::void_t<
                  decltype(std::declval<ValAdaptor>().operator T())>>
    std::vector<T> asVector() const {
        return std::vector<T>(vec.begin(), vec.end());
    }
    template <typename T,
              typename = std::void_t<
                  decltype(std::declval<ValAdaptor>().operator T())>>
    operator std::vector<T>() const {
        return asVector<T>();
    }
    template <typename T, std::size_t N,
              typename = std::void_t<
                  decltype(std::declval<ValAdaptor>().operator T())>>
    std::array<T, N> asArray() const {
        if (N != vec.size()) {
            throw std::invalid_argument(
                "array size mismatch, expected: " + std::to_string(N) +
                ", got: " + std::to_string(vec.size()));
        }
        std::array<T, N> a;
        for (std::size_t i = 0; i < N; i++) {
            a[i] = vec[i].as<T>();
        }
        return a;
    }
    template <typename T, std::size_t N,
              typename = std::void_t<
                  decltype(std::declval<ValAdaptor>().operator T())>>
    operator std::array<T, N>() const {
        return asArray<T, N>();
    }

    template <typename T>
    T as() const {
        if constexpr (std::is_same_v<T, ValAdaptorVector>) {
            return *this;
        } else {
            return this->operator T();
        }
    }

    const ValAdaptor &operator[](std::size_t index) const { return at(index); }
    const ValAdaptor &at(std::size_t index) const;

    const ValAdaptor *data() const { return &at(0); }
    const ValAdaptor *begin() const { return &at(0); }
    const ValAdaptor *end() const { return begin() + size(); }
    const ValAdaptor *cbegin() const { return &at(0); }
    const ValAdaptor *cend() const { return begin() + size(); }

    std::size_t size() const;

    [[deprecated("(ver2.10〜) use asStringView() or asString() instead")]]
    std::string asStringRef() const {
        return asString();
    }
    [[deprecated("(ver2.10〜) use asWStringView() or asWString() instead")]]
    std::wstring asWStringRef() const {
        return asWString();
    }
    StringView asStringView() const { return get().asStringView(); }
    WStringView asWStringView() const { return get().asWStringView(); }
    std::string_view asU8StringView() const { return get().asU8StringView(); }
    std::string asString() const { return get().asString(); }
    std::wstring asWString() const { return get().asWString(); }
    double asDouble() const { return get().asDouble(); }
    int asInt() const { return get().asInt(); }
    long long asLLong() const { return get().asLLong(); }
    bool asBool() const { return get().asBool(); }

    bool operator==(const ValAdaptorVector &other) const;
    bool operator!=(const ValAdaptorVector &other) const {
        return !(*this == other);
    }

    template <typename T, typename std::enable_if_t<
                              std::is_constructible_v<ValAdaptorVector, T> &&
                                  !std::is_same_v<ValAdaptorVector, T>,
                              std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return *this == ValAdaptorVector(other);
    }
    template <typename T, typename std::enable_if_t<
                              std::is_constructible_v<ValAdaptorVector, T> &&
                                  !std::is_same_v<ValAdaptorVector, T>,
                              std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return *this != ValAdaptorVector(other);
    }
};

template <typename T, typename std::enable_if_t<
                          std::is_constructible_v<ValAdaptorVector, T> &&
                              !std::is_same_v<ValAdaptorVector, T>,
                          std::nullptr_t> = nullptr>
bool operator==(const T &other, const ValAdaptorVector &val) {
    return val == ValAdaptorVector(other);
}
template <typename T, typename std::enable_if_t<
                          std::is_constructible_v<ValAdaptorVector, T> &&
                              !std::is_same_v<ValAdaptorVector, T>,
                          std::nullptr_t> = nullptr>
bool operator!=(const T &other, const ValAdaptorVector &val) {
    return val != ValAdaptorVector(other);
}

/*!
 * \brief ValAdaptorのリストから任意の型のタプルに変換する
 *
 * ver2.10〜 ValAdaptorVectorに変更
 *
 */
template <int n = 0, typename T>
void argToTuple(const std::vector<ValAdaptorVector> &args, T &tuple) {
    constexpr int tuple_size = std::tuple_size<T>::value;
    if constexpr (n < tuple_size) {
        using Type = typename std::tuple_element<n, T>::type;
        std::get<n>(tuple) = args[n].as<Type>();
        argToTuple<n + 1>(args, tuple);
    }
}


WEBCFACE_NS_END
