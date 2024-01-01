module;

export module Game.Pool;

import Container.Pool;
import <unordered_map>;
import <typeindex>;
import <any>;
import <memory>;

namespace Game::Pool{
	std::unordered_map<std::type_index, std::any> pools(100);
}

export namespace Game::Pool{
	template <typename T>
	Containers::Pool<T>& getPool(size_t initSize = 5000) {
		const auto index = std::type_index{typeid(T)};

		auto itr = pools.find(index);
		if(itr == pools.end()) {
			itr = pools.emplace(index, Containers::Pool<T>{initSize}).first;
		}

		return std::any_cast<Containers::Pool<T>>(itr->second);
	}

	template <typename T>
	std::shared_ptr<T> obtainShared() {
		return getPool<T>().obtainUnique();
	}

	template <typename T>
	std::unique_ptr<T> obtainUnique() {
		return getPool<T>().obtainShared();
	}

	template <typename T>
	T* obtainRaw() {
		return getPool<T>().obtainRaw();
	}
}
