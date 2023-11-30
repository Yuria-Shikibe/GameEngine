//
// Created by Matrix on 2023/11/18.
//

module ;

export module Container.Pool;

import <functional>;
import <memory>;
import <vector>;

import Concepts;

export namespace Containers{
    template <class T>
        requires Concepts::HasDefConstructor<T>
    class Pool{
    public:
        struct PoolDeleter {
            Pool* src{ nullptr };
            std::function<void(T*)> reset{ nullptr };

            ~PoolDeleter() = default;

            PoolDeleter() = default;

            PoolDeleter(const PoolDeleter& other)
                    : src(other.src),
                      reset(other.reset){
            }

            PoolDeleter(PoolDeleter&& other) noexcept
                    : src(other.src),
                      reset(std::move(other.reset)){
            }

            PoolDeleter& operator=(const PoolDeleter& other){
                if(this == &other) return *this;
                src = other.src;
                reset = other.reset;
                return *this;
            }

            PoolDeleter& operator=(PoolDeleter&& other) noexcept{
                if(this == &other) return *this;
                src = other.src;
                reset = std::move(other.reset);
                return *this;
            }

            explicit PoolDeleter(Pool* src)
                    : src(src) {
            }

            PoolDeleter(Pool* src, const std::function<void(T*)>& reset)
                    : src(src),
                      reset(reset){
            }

            void operator()(T* t) const {
                if(reset)reset(t);
                src->store(t);
            }
        };

    protected:
        size_t maxSize{0};
        std::vector<std::unique_ptr<T>> vault;
        PoolDeleter deleter;

    public:
        explicit Pool(std::function<void(T*)> func) : deleter(PoolDeleter{this, func}) {

        }

        Pool() : deleter(PoolDeleter{this}){

        }

        void store(std::unique_ptr<T>&& ptr){
            T* t = ptr.release();

            vault.push_back(std::make_unique<T>(*t));
        }

        void store(T* ptr) {
            vault.push_back(std::make_unique<T>(*ptr));
        }

        std::unique_ptr<T, PoolDeleter> obtainUnique() {
            if (vault.empty()) {
                return std::unique_ptr<T, PoolDeleter>{new T, deleter};
            }

            T* ptr = vault.back().release();
            vault.pop_back();

            return std::unique_ptr<T, PoolDeleter>{ptr, deleter};
        }

        std::shared_ptr<T> obtainShared() {
            if (vault.empty()) {
                return std::shared_ptr<T>{new T, deleter};
            }

            T* ptr = vault.back().release();
            vault.pop_back();

            return std::shared_ptr<T>{ptr, deleter};
        }

        [[nodiscard]] size_t size() const{
            return vault.size();
        }

        void clear(){
            vault.clear();
        }
    };
}

