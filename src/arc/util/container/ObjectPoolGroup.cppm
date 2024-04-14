export module ext.Container.ObjectPoolGroup;

export import ext.Container.ObjectPool;
import ext.RuntimeException;
import std;

export namespace ext{
	class ObjectPoolGroup{
		struct PoolWrapper {
			void* poolPtr{nullptr};
			void(* destructor)(void*){nullptr};

			explicit PoolWrapper(void(* destructor)(void*))
				: destructor{destructor}{}

			~PoolWrapper(){
				destruct();
			}

			PoolWrapper(const PoolWrapper& other) = delete;

			PoolWrapper(PoolWrapper&& other) noexcept
				: poolPtr(other.poolPtr),
				  destructor(other.destructor){
				other.poolPtr = nullptr;
			}

			PoolWrapper& operator=(const PoolWrapper& other) = delete;

			PoolWrapper& operator=(PoolWrapper&& other) noexcept{
				if(this == &other) return *this;
				destruct();

				poolPtr = other.poolPtr;
				destructor = other.destructor;
				other.poolPtr = nullptr;

				return *this;
			}


			template <typename T, typename ...Args>
			void construct(Args... args)
			{
				destruct();
				poolPtr = new ext::ObjectPool<T>{args...};
			}

			void destruct(){
				destructor(poolPtr);
				poolPtr = nullptr;
			}

			template <typename T>
			ext::ObjectPool<T>* as() {
				return static_cast<ext::ObjectPool<T>*>(poolPtr);
			}
		};

		template <typename T>
		static void destruct(void* ptr) noexcept{
			delete static_cast<ext::ObjectPool<T>*>(ptr);
		}

		std::unordered_map<std::type_index, PoolWrapper> pools{100};

	public:
		template <typename T>
		ext::ObjectPool<T>& getPool(const size_t initSize = 5000){
			const auto index = std::type_index{typeid(T)};

			auto itr = pools.find(index);
			if(itr == pools.end()){
				bool successed{false};
				std::tie(itr, successed) = pools.try_emplace(index, PoolWrapper{destruct<T>});
				if(!successed){
					throw ext::RuntimeException{"Failed ObjectPool Construct!"};
				}
				itr->second.construct<T>(initSize);
			}

			return *itr->second.as<T>();
		}

		template <typename T>
		std::shared_ptr<T> obtainShared(){
			return getPool<T>().obtainShared();
		}

		template <typename T>
		typename ext::ObjectPool<T>::UniquePtr obtainUnique(){
			return getPool<T>().obtainUnique();
		}

		template <typename T>
		T* obtainRaw(){
			return getPool<T>().obtainRaw();
		}
	};
}
