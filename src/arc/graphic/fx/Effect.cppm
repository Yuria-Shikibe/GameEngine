//
// Created by Matrix on 2024/3/8.
//

export module Graphic.Effect;

export import Math.Timed;

import Geom.Transform;
import Graphic.Color;
import ext.Concepts;
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
		float defLifetime = 60.0f;
		virtual ~EffectDrawer() = default;
		virtual void operator()(Effect& effect) const = 0;

		constexpr EffectDrawer() = default;

		constexpr explicit EffectDrawer(const float defaultLifetime)
			: defLifetime(defaultLifetime){}

		Effect* suspendOn(EffectManager* manager) const;
	};

	template<Concepts::Invokable<void(Graphic::Effect&)> Func>
	struct EffectDrawer_Func final : EffectDrawer{
		Func func{};

		explicit EffectDrawer_Func(const float time, Func&& func)
			: EffectDrawer(time), func(std::forward<Func>(func)){}

		void operator()(Effect& effect) const override{
			func(effect);
		}
	};

	template <Concepts::Invokable<void(Graphic::Effect&)> Func>
	EffectDrawer_Func(float, Func) -> EffectDrawer_Func<Func>;

	template <Concepts::Invokable<void(Graphic::Effect&)> Func>
	std::unique_ptr<EffectDrawer_Func<Func>> makeEffect(const float time, Func&& func){
		return std::make_unique<EffectDrawer_Func<Func>>(time, std::forward<Func>(func));
	}

	struct EffectDrawer_Multi final : EffectDrawer{
		std::vector<const EffectDrawer*> subEffects{};

		explicit EffectDrawer_Multi(const std::initializer_list<const EffectDrawer*> effects)
			: subEffects(effects){}

		void operator()(Effect& effect) const override{
			for (const auto subEffect : this->subEffects){
				subEffect->operator()(effect);
			}
		}
	};

	struct Effect {
		using HandleType = size_t;
		static constexpr float DefLifetime = -1.0f;

		Math::Timed progress{};

		float zOffset{3};
		Geom::Transform trans{};
		HandleType handle{};

		Color color{};
		std::any additionalData{};

		const EffectDrawer* drawer{nullptr};

		Effect* set(const Geom::Transform trans, const Color color = Colors::WHITE, const float lifetime = DefLifetime, std::any&& additonalData = {}){
			if(lifetime > 0.0f){
				progress.lifetime = lifetime;
			}else{
				if(drawer)progress.lifetime = drawer->defLifetime;
			}

			this->trans = trans;
			this->color = color;
			this->additionalData = std::move(additonalData);

			return this;
		}

		Effect* setLifetime(const float lifetime = DefLifetime){
			if(lifetime > 0.0f){
				progress.lifetime = lifetime;
			}else{
				if(drawer)progress.lifetime = drawer->defLifetime;
			}
			return this;
		}

		Effect* setColor(const Color& color = Colors::WHITE){
			this->color = color;
			return this;
		}

		Effect* setDrawer(const EffectDrawer* drawer){
			this->drawer = drawer;
			this->progress.set(0.0f, drawer->defLifetime);
			return this;
		}

		void resignHandle(){
			handle = allocateHandle();
		}

		bool overrideRotation(HandleType& handle, const float rotation){
			if(handle == this->handle){
				this->trans.rot = rotation;
				return true;
			}else{
				handle = 0;
				return false;
			}
		}

		bool overridePosition(HandleType& handle, const Geom::Vec2 position){
			if(handle == this->handle){
				this->trans.vec = position;
				return true;
			}else{
				handle = 0;
				return false;
			}
		}

		[[nodiscard]] bool removalble() const{
			return progress.time >= progress.lifetime;
		}

		[[nodiscard]] float getX() const{
			return trans.vec.x;
		}

		[[nodiscard]] float getY() const{
			return trans.vec.y;
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
