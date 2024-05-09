export module Graphic.Effect.Manager;

export import Graphic.Effect;

import ext.Container.ObjectPool;
import ext.Heterogeneous;

import std;

export namespace Graphic{
	/**
	 * @brief Basically, if no exception happens, once a effect is created, it won't be deleted!
	 */
	class EffectManager {
		static constexpr size_t MaxEffectBufferSize = 4096;

		using PoolType = ext::ObjectPool<Effect>;
		PoolType effectPool{};

		ext::UniquePtrSet<Effect, PoolType::Deleter> activatedEffects{MaxEffectBufferSize};

		std::vector<Effect*> toRemove{};
		std::stack<PoolType::UniquePtr> waiting{};

		std::mutex suspendLock{};

	public:
		void update(const float delta){
			toRemove.clear();

			for(auto& effect : activatedEffects){
				if(effect->update(delta)){
					toRemove.push_back(effect.get());
				}
			}

			for(const auto& effect : toRemove){
				if(const auto& itr = activatedEffects.extract(effect)){
					waiting.push(std::move(itr.value()));
				}
			}
		}

		[[nodiscard]] Effect* suspend(){
			Effect* out;

			{
				PoolType::UniquePtr ptr;
				std::scoped_lock gurad{suspendLock};
				if(waiting.empty()){
					ptr = effectPool.obtainUnique();
				}else{
					ptr = std::move(waiting.top());
					waiting.pop();
				}

				auto [kv, success] = activatedEffects.insert(std::move(ptr));
				out = kv->get();
			}

			out->progress.time = 0;
			out->resignHandle();

			return out;
		}

		void render() const{
			for(auto& effect : activatedEffects){
				effect->render();
			}
		}
	};
}
