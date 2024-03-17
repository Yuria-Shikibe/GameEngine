export module Graphic.Effect.Manager;

export import Graphic.Effect;

import Container.Pool;

import std;

export namespace Graphic{
	/**
	 * @brief Basically, if no exception happens, once a effect is created, it won't be deleted!
	 */
	class EffectManager {
		static constexpr size_t MaxEffectBufferSize = 4096;

		using PoolType = Containers::Pool<Effect>;
		PoolType effectPool{};

		//yes this is bad
		std::unordered_map<Effect*, PoolType::UniquePtr> activatedEffects{MaxEffectBufferSize};

		std::vector<Effect*> toRemove{};
		std::stack<PoolType::UniquePtr> waiting{};

		std::mutex suspendLock{};

	public:
		void update(const float delta){
			toRemove.clear();

			for(auto& effect : activatedEffects | std::ranges::views::values){
				if(effect->update(delta)){
					toRemove.push_back(effect.get());
				}
			}

			for(const auto& effect : toRemove){
				const auto itr = activatedEffects.find(effect);
				waiting.push(std::move(itr->second));
				activatedEffects.erase(itr);
			}
		}

		[[nodiscard]] Effect* suspend(){
			Effect* out{nullptr};

			{
				PoolType::UniquePtr ptr;
				std::lock_guard gurad{suspendLock};
				if(waiting.empty()){
					ptr = effectPool.obtainUnique();
				}else{
					ptr = std::move(waiting.top());
					waiting.pop();
				}

				auto [kv, success] = activatedEffects.try_emplace(ptr.get(), std::move(ptr));
				out = kv->second.get();
			}

			out->progress.time = 0;
			out->resignHandle();

			return out;
		}

		void render() const{
			for(auto& effect : activatedEffects | std::ranges::views::values){
				effect->render();
			}
		}
	};
}
