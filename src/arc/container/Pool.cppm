//
// Created by Matrix on 2023/11/18.
//

module ;

export module Container.Pool;

import <memory_resource>;
import <functional>;
import <memory>;
import <vector>;

import Concepts;


export namespace Containers{
    template <class T>
        requires Concepts::HasDefConstructor<T>
    class Pool{
    public:
        struct Deleter {
            Pool* const src{ nullptr };
            std::function<void(T*)> reset{ nullptr };

            ~Deleter() = default;

            Deleter() = default;

            Deleter(const Deleter& other)
                    : src(other.src),
                      reset(other.reset){
            }

            Deleter(Deleter&& other) noexcept
                    : src(other.src),
                      reset(std::move(other.reset)){
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
                reset = std::move(other.reset);
                return *this;
            }

            explicit Deleter(Pool* src)
                    : src(src) {
            }

            Deleter(Pool* src, std::function<void(T*)>&& reset)
                    : src(src),
                      reset(reset){
            }

            void operator()(T* t) const {
                if(reset)reset(t);
                src->store(t);
            }
        };

        struct Allocator : ::std::allocator<T*> {
            Pool* const src{ nullptr };

            T** allocate() {
                return &src->obtainRaw();
            }

            void deallocate(T** ptr, size_t n) {
                src->store(*ptr);
            }
        };

    protected:
        size_t maxSize{2000};
        std::vector<T*> vault;
        Deleter deleter;

        std::unique_ptr<std::pmr::monotonic_buffer_resource> monotonicPool{nullptr};

    public:
        [[nodiscard]] Pool(const size_t maxSize, std::function<void(T*)>&& func)
            : maxSize(maxSize),
            vault(vault), deleter(Deleter{this, std::forward<std::function<void(T*)>>(func)})  {
            vault.reserve(maxSize);
        }

        explicit Pool(std::function<void(T*)>&& func) : deleter(Deleter{this, std::forward<std::function<void(T*)>>(func)}) {
            vault.reserve(maxSize);
        }

        explicit Pool(const size_t maxSize) : maxSize(maxSize), deleter(Deleter{this}){
            vault.reserve(maxSize);
        }

        Pool() : deleter(Deleter{this}){
            vault.reserve(maxSize);
        }

        ~Pool() {
            while(!vault.empty()) {
                delete vault.back();
                vault.pop_back();
            }
        }

        void store(std::unique_ptr<T>&& ptr){
            if(vault.size() < maxSize){
                vault.push_back(ptr.release());
            }else {
                ptr.reset(nullptr);
            }
        }

        void store(T* ptr) {
            if(vault.size() < maxSize){
                vault.push_back(ptr);
            }else {
                delete ptr;
            }
        }

        std::pmr::monotonic_buffer_resource* getMonotonicPool() {
            if(monotonicPool) {
                return monotonicPool.get();
            }

            monotonicPool = std::make_unique<std::pmr::monotonic_buffer_resource>(vault.data(), vault.capacity());
            return monotonicPool.get();
        }

        Allocator getAllocator() const {
            return Allocator{this};
        }

        std::unique_ptr<T, Deleter> obtainUnique() {
            if (vault.empty()) {
                return std::unique_ptr<T, Deleter>{new T, deleter};
            }

            T* ptr = vault.back();
            vault.pop_back();

            return std::unique_ptr<T, Deleter>{ptr, deleter};
        }

        std::shared_ptr<T> obtainShared() {
            if (vault.empty()) {
                return std::shared_ptr<T>{new T, deleter};
            }

            T* ptr = vault.back();
            vault.pop_back();

            return std::shared_ptr<T>{ptr, deleter};
        }

        std::shared_ptr<T> obtainRaw() {
            if (vault.empty()) {
                return new T;
            }

            T* ptr = vault.back();
            vault.pop_back();

            return ptr;
        }

        [[nodiscard]] size_t size() const{
            return vault.size();
        }

        void clear(){
            vault.clear();
        }
    };
}

