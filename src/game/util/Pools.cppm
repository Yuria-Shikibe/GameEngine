module;

export module Game.Pool;

import Concepts;
export import Container.Pool;
import RuntimeException;
import <unordered_map>;
import <typeindex>;
import <any>;
import <memory>;

namespace Game::Pools{
	struct PoolWrapper {
		/**
		 * \brief Once added, no delete. This pool will survive until the application process is killed!
		 */
		void* pool{nullptr};

		template <typename T, typename ...Args>
		void construct(Args... args)
		{
			this->pool = new Containers::Pool<T>{args...};
		}

		template <typename T>
		Containers::Pool<T>* as() {
			return static_cast<Containers::Pool<T>*>(pool);
		}
	};

	std::unordered_map<std::type_index, PoolWrapper> pools(100);
}

export namespace Game::Pools{
	template <typename T>
	Containers::Pool<T>* getPool(const size_t initSize = 5000) {
		const auto index = std::type_index{typeid(T)};

		auto itr = pools.find(index);
		if(itr == pools.end()) {
			bool successed;
			std::tie(itr, successed) = pools.try_emplace(index, PoolWrapper{});
			if(!successed) {
				throw ext::RuntimeException{"Failed Pool Construct!"};
			}
			itr->second.construct<T>(initSize);
		}

		return itr->second.as<T>();
	}

	template <typename T>
	std::shared_ptr<T> obtainShared() {
		return getPool<T>()->obtainShared();
	}

	template <typename T>
	typename Containers::Pool<T>::UniquePtr obtainUnique() {
		return getPool<T>()->obtainUnique();
	}

	template <typename T>
	T* obtainRaw() {
		return getPool<T>()->obtainRaw();
	}
}
