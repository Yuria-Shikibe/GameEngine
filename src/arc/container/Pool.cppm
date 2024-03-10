module ;

export module Container.Pool;

import <memory_resource>;
import <functional>;
import <memory>;
import <vector>;

import Concepts;
import RuntimeException;


export namespace Containers{
    //TODO this class is a totoally failure
    template <class T>
        requires Concepts::DefConstructable<T>
    class Pool{
    public:
        using ItemRef = T*;
        struct Deleter {
            Pool* src{ nullptr };
            std::function<void(ItemRef)> reset{ nullptr };

            ~Deleter() = default;

            Deleter() = default;

            Deleter(const Deleter& other)
                    : src(other.src),
                      reset(other.reset){
            }

            Deleter(Deleter&& other) noexcept
                    : src(other.src),
                      reset(std::move(other.reset)){
                other.src = nullptr;
            }

            Deleter& operator=(const Deleter& other){
                if(this == &other) return *this;
                src = other.src;
                reset = other.reset;
                return *this;
            }

            Deleter& operator=(Deleter&& other) noexcept{
                if(this == &other) return *this;
                src = other.src;
                other.src = nullptr;
                reset = std::move(other.reset);
                return *this;
            }

            explicit Deleter(Pool* src)
                    : src(src) {
            }

            Deleter(Pool* src, std::function<void(ItemRef)>&& reset)
                    : src(src),
                      reset(reset){
            }

            void operator()(ItemRef t) const {
                if(!src) {
                    throw ext::NullPointerException{"Source pool is expired!"};
                }
                if(reset)reset(t);
                src->store(t);
            }
        };

        struct Allocator_Raw : ::std::allocator<ItemRef> {
            Pool* const src{ nullptr };

            ItemRef* allocate(const size_t count) {
                return &src->obtainRaw();
            }

            void deallocate(ItemRef* ptr, size_t n) {
                src->store(*ptr);
            }
        };

        struct Allocator_Shared : ::std::allocator<std::shared_ptr<T>> {
            Pool* const src{ nullptr };

            std::shared_ptr<T>* allocate(const size_t count) {
                return &src->obtainShared();
            }

            void deallocate(std::shared_ptr<T>* ptr, size_t n) {
                ptr->reset(nullptr);
            }
        };

        struct Allocator_Unique : ::std::allocator<std::unique_ptr<T>> {
            Pool* const src{ nullptr };

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
        [[nodiscard]] Pool(const size_t maxSize, std::function<void(ItemRef)>&& func)
            : maxSize(maxSize),
            vault(vault), deleter(Deleter{this, std::forward<std::function<void(ItemRef)>>(func)})  {
            vault.reserve(maxSize);
        }

        explicit Pool(std::function<void(ItemRef)>&& func) : deleter(Deleter{this, std::forward<decltype(func)>(func)}) {
            vault.reserve(maxSize);
        }

        explicit Pool(const size_t maxSize) : maxSize(maxSize), deleter(Deleter{this}){
            vault.reserve(maxSize);
        }

        Pool(const Pool& other) = delete;

        Pool& operator=(const Pool& other) = delete;

        Pool(Pool&& other) noexcept = default;

        Pool& operator=(Pool&& other) noexcept = default;

        Pool() : deleter(Deleter{this}){
            vault.reserve(maxSize);
        }

        ~Pool() {
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
                ptr.reset(nullptr);
            }
        }

        void store(ItemRef ptr) {
            std::lock_guard guard{vaultLock};

            if(vault.size() < maxSize){
                vault.push_back(ptr);
            }else {
                delete ptr;
            }
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
            if (vault.empty()) {
                return UniquePtr{new T, deleter};
            }

            ItemRef ptr;

            {
                std::lock_guard guard{vaultLock};
                ptr = vault.back();
                vault.pop_back();
            }

            return std::unique_ptr<T, Deleter>{ptr, deleter};
        }

        [[nodiscard]] std::shared_ptr<T> obtainShared() {

            if (vault.empty()) {
                return std::shared_ptr<T>{new T, deleter};
            }

            ItemRef ptr = nullptr;

            {
                std::lock_guard guard{vaultLock};
                ptr = vault.back();
                vault.pop_back();
            }

            return std::shared_ptr<T>{ptr, deleter};
        }

        [[nodiscard]] ItemRef obtainRaw() {
            if (vault.empty()) {
                return new T;
            }

            ItemRef ptr = nullptr;

            {
                std::lock_guard guard{vaultLock};
                ptr = vault.back();
                vault.pop_back();
            }

            return ptr;
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

