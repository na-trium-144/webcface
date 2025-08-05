#pragma once
#include <memory>
#include <memory_resource>
#include <type_traits>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
/*!
 * \brief Shared Memory Resource
 */
namespace smr {

template <typename T>
class SharedResourceAllocator;

WEBCFACE_DLL SharedResourceAllocator<char> WEBCFACE_CALL getStaticAllocator();

/*!
 * \brief memory_resourceのshared_ptrを保持するアロケータ
 * \since ver2.10
 *
 * * デフォルトのpool_resourceの寿命の範囲内では、pool_resourceのshared_ptrを共有するAllocatorを使う。
 * * pool_resourceの寿命が切れている場合、std::pmr::new_delete_resourceにフォールバックする。
 * * 実際の初期化の実装は getStaticAllocator() にある
 * (テンプレートではソースファイルに置けないので)
 *
 */
template <typename T>
class SharedResourceAllocator : public std::pmr::polymorphic_allocator<T> {
    std::shared_ptr<std::pmr::memory_resource> sr;

  public:
    SharedResourceAllocator(std::pmr::memory_resource *r,
                            std::shared_ptr<std::pmr::memory_resource> sr)
        : std::pmr::polymorphic_allocator<T>(r), sr(std::move(sr)) {}

    SharedResourceAllocator(const SharedResourceAllocator &other) = default;
    SharedResourceAllocator &
    operator=(const SharedResourceAllocator &other) = delete;
    ~SharedResourceAllocator() = default;

    /*!
     * \brief 型変換コンストラクタ
     *
     */
    template <typename U>
    SharedResourceAllocator(const SharedResourceAllocator<U> &other)
        : std::pmr::polymorphic_allocator<T>(other.resource()), sr(other.sr) {}

    /*!
     * \brief デフォルトコンストラクタ
     *
     * getStaticAllocator() で得られるallocatorで初期化する
     *
     */
    SharedResourceAllocator() : SharedResourceAllocator(getStaticAllocator()) {}

    /*!
     * コンテナのコピー構築時にアロケータをどうするかを定義
     *
     */
    SharedResourceAllocator select_on_container_copy_construction() const {
        return *this;
    }
};

/*!
 * \brief unique_ptrで使用するdeleter
 * \since ver2.10
 */
template <typename T>
class SharedResourceDeleter {
    SharedResourceAllocator<T> alloc;
    static_assert(!std::is_array_v<T>,
                  "SharedResourceDeleter for T[] is not implemented yet");

  public:
    explicit SharedResourceDeleter(const SharedResourceAllocator<T> &alloc)
        : alloc(alloc) {}
    template <class U>
    SharedResourceDeleter(const SharedResourceDeleter<U> &other)
        : alloc(other.alloc) {}
    void operator()(T *ptr) const noexcept {
        alloc.destroy(ptr);
        alloc.deallocate(ptr, 1);
    }
};

} // namespace smr
WEBCFACE_NS_END
