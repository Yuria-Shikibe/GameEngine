export module ext.Container.ObjectPool;

import std;

import Concepts;
import ext.RuntimeException;


export namespace ext{
    //TODO this class is a totoally failure
    template <class T>
        requires Concepts::DefConstructable<T>
    class ObjectPool{
    public:
        using ItemRef = T*;
        struct Deleter {
            ObjectPool* src{ nullptr };

            ~Deleter() = default;

            Deleter() = default;

            explicit Deleter(ObjectPool* src)
                    : src(src) {
            }

            void operator()(const ItemRef t) const {
                if(!src) {
                    delete t;
                }else{
                    src->store(t);
                }
            }
        };

        struct Allocator_Raw : ::std::allocator<ItemRef> {
            ObjectPool* const src{ nullptr };

            ItemRef* allocate(const size_t count) {
                return &src->obtainRaw();
            }

            void deallocate(ItemRef* ptr, size_t n) {
                src->store(*ptr);
            }
        };

        struct Allocator_Shared : ::std::allocator<std::shared_ptr<T>> {
            ObjectPool* const src{ nullptr };

            std::shared_ptr<T>* allocate(const size_t count) {
                return &src->obtainShared();
            }

            void deallocate(std::shared_ptr<T>* ptr, size_t n) {
                ptr->reset(nullptr);
            }
        };

        struct Allocator_Unique : ::std::allocator<std::unique_ptr<T>> {
            ObjectPool* const src{ nullptr };

            std::unique_ptr<T>* allocate(const size_t count) {
                return &src->obtainUnique();
            }

            void deallocate(std::unique_ptr<T>* ptr, size_t n) {
                src->store(std::move(*ptr));
            }
        };

    protected:
        size_t maxSize{5000};
        std::vector<ItemRef> vault;

        Deleter deleter;
        std::mutex vaultLock{};
        // std::unique_ptr<std::pmr::monotonic_buffer_resource> monotonicPool{nullptr};

    public:
        Deleter getDeleter(){
            return deleter;
        }

        using UniquePtr = std::unique_ptr<T, Deleter>;

        explicit ObjectPool(const size_t maxSize) : maxSize(maxSize), deleter(Deleter{this}){
            vault.reserve(maxSize);
        }

        ObjectPool(const ObjectPool& other) = delete;

        ObjectPool& operator=(const ObjectPool& other) = delete;

        ObjectPool(ObjectPool&& other) noexcept = default;

        ObjectPool& operator=(ObjectPool&& other) noexcept = default;

        ObjectPool() : deleter(Deleter{this}){
            vault.reserve(maxSize);
        }

        ~ObjectPool() {
            while(!vault.empty()) {
                delete vault.back();
                vault.pop_back();
            }
        }

        void store(std::shared_ptr<T>& ptr) {
            //TODO does this safe>>
            if(std::addressof(get_deleter(ptr)) == &deleter) {
                std::lock_guard guard{vaultLock};
                ptr.reset(nullptr);
            }
        }

        void store(std::unique_ptr<T>&& ptr){
            std::lock_guard guard{vaultLock};

            if(vault.size() < maxSize){
                vault.push_back(ptr.release());
            }else {
                ptr.reset();
            }
        }

        void store(ItemRef ptr) {
            if(std::lock_guard guard{vaultLock}; vault.size() < maxSize){
                ptr->~T();
                vault.push_back(ptr);
                return;
            }

            delete ptr;
        }

        // std::pmr::monotonic_buffer_resource* getMonotonicPool() {
        //     if(monotonicPool) {
        //         return monotonicPool.get();
        //     }
        //
        //     monotonicPool = std::make_unique<std::pmr::monotonic_buffer_resource>(vault.data(), vault.capacity());
        //     return monotonicPool.get();
        // }

        Allocator_Raw getAllocator_Raw() const {
            return Allocator_Raw{this};
        }

        Allocator_Shared getAllocator_Shared() const {
            return Allocator_Shared{this};
        }

        Allocator_Unique getAllocator_Unique() const {
            return Allocator_Unique{this};
        }

        [[nodiscard]] UniquePtr obtainUnique() {
            return UniquePtr{obtainRaw(), deleter};
        }

        [[nodiscard]] ItemRef obtainRaw() {
            if (vault.empty()) {
                return new T;
            }

            ItemRef ptr = nullptr;

            {
                std::lock_guard guard{vaultLock};
                ptr = vault.back();
                new (ptr) T{};
                vault.pop_back();
            }

            return ptr;
        }


        [[nodiscard]] std::shared_ptr<T> obtainShared() {
            return std::shared_ptr<T>{obtainRaw(), deleter};
        }

        [[nodiscard]] size_t size() const{
            std::lock_guard guard{vaultLock};
            return vault.size();
        }

        void clear(){
            std::lock_guard guard{vaultLock};
            vault.clear();
        }
    };
}

