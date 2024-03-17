//
// Created by Matrix on 2024/3/8.
//

export module Graphic.Effect;

export import Graphic.Timed;

import Geom.Vector2D;
import Graphic.Color;
import Concepts;
import std;

namespace Graphic{
	inline std::atomic_size_t next = 0;
	size_t allocateHandle(){
		++next;
		return next;
	}
}

export namespace Graphic{
	struct Effect;
	class EffectManager;

	struct EffectDrawer{
		virtual ~EffectDrawer() = default;
		virtual void operator()(Effect& effect) = 0;

		Effect* suspendOn(EffectManager* manager);
	};

	template<Concepts::Invokable<void(Effect&)> auto func>
	struct EffectDrawer_Func final : EffectDrawer{
		void operator()(Effect& effect) override{
			func(effect);
		}
	};

	struct Effect {
		using HandleType = size_t;

		Timed progress{};

		Geom::Vec2 position{};
		float rotation{};
		HandleType handle{};

		Color color{};
		std::any additionalData{};

		EffectDrawer* drawer{nullptr};

		Effect& operator=(const Effect& other){
			if(this == &other) return *this;
			progress = other.progress;
			position = other.position;
			rotation = other.rotation;
			handle = other.handle;
			color = other.color;
			additionalData = other.additionalData;
			drawer = other.drawer;
			return *this;
		}

		Effect& operator=(Effect&& other) noexcept{
			if(this == &other) return *this;
			progress = other.progress;
			position = other.position;
			rotation = other.rotation;
			handle = other.handle;
			color = other.color;
			additionalData = std::move(other.additionalData);
			drawer = other.drawer;
			return *this;
		}

		Effect* set(const Geom::Vec2 position, const float rotation = 0.0f, const Color color = Colors::WHITE, const float lifetime = 60.0f, std::any&& additonalData = {}){
			progress.set(0.0f, lifetime);
			this->position = position;
			this->rotation = rotation;
			this->color = color;
			this->additionalData = std::move(additonalData);

			return this;
		}

		Effect* setDrawer(EffectDrawer* drawer){
			this->drawer = drawer;
			return this;
		}

		void resignHandle(){
			handle = allocateHandle();
		}

		bool overrideRotation(HandleType& handle, const float rotation){
			if(handle == this->handle){
				this->rotation = rotation;
				return true;
			}else{
				handle = 0;
				return false;
			}
		}

		bool overridePosition(HandleType& handle, const Geom::Vec2 position){
			if(handle == this->handle){
				this->position = position;
				return true;
			}else{
				handle = 0;
				return false;
			}
		}

		[[nodiscard]] bool removalble() const{
			return progress.time >= progress.lifetime;
		}


		/**
		 * @return Whether this effect is removeable
		 */
		[[nodiscard]] bool update(const float delta){
			progress.time += delta;
			if(progress.time >= progress.lifetime){
				progress.time = progress.lifetime;
				return true;
			}

			return false;
		}

		void render(){
			drawer->operator()(*this);
		}
	};
}
