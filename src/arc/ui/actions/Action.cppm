//
// Created by Matrix on 2024/3/30.
//

export module UI.Action;

export import Math.Timed;
export import Math.Interpolation;
export import ext.RuntimeException;
import std;

// export import Container.Pool;

export namespace UI{

	//TODO is an object pool needed?

	template <typename T>
	struct Action{
		virtual ~Action() = default;

		Math::Timed scale{};
		const Math::Interp::InterpFunc interpFunc{Math::Interp::linear};

		Action() = default;

		Action(const float lifetime, const Math::Interp::InterpFunc& interpFunc)
			: scale(0, lifetime),
			  interpFunc(interpFunc){}

		explicit Action(const float lifetime)
			: scale(0, lifetime){}

		[[nodiscard]] Math::Interp::InterpFunc getInterp() const{
#ifdef _DEBUG
			if(!interpFunc){
				throw ext::NullPointerException{"Interp Func is null!"};
			}
#endif

			return interpFunc;
		}

		/**
		 * @return The excess time, it > 0 means this action has been done
		 */
		virtual float update(const float delta, T* elem){
			float ret = -1.0f;

			if(scale.time > scale.lifetime)return delta;

			if(scale.time == 0.0f){
				this->begin(elem);
				scale.time = std::numeric_limits<float>::min();
			}

			scale.time += delta;


			if(scale.time >= scale.lifetime){
				ret = scale.time - scale.lifetime;
				scale.time = scale.lifetime;
			}


			this->apply(elem, scale.get(getInterp()));

			if(ret >= 0.0f){
				this->end(elem);
			}

			return ret;
		}

		virtual void apply(T* elem, float progress){}
		virtual void begin(T* elem){}
		virtual void end(T* elem){}
	};

	template <typename T>
	struct ParallelAction : Action<T>{
		using Action<T>::scale;
		using Action<T>::getInterp;

	protected:
		void alignMaxHealth(){
			for(auto& action : actions){
				scale.lifetime = std::max(scale.lifetimem, action->scale.lifetime);
			}
		}

	public:
		std::vector<std::unique_ptr<Action<T>>> actions{};

		explicit ParallelAction(const std::vector<std::unique_ptr<Action<T>>>& actions)
			: actions(actions){
			alignMaxHealth();
		}

		ParallelAction(const std::initializer_list<std::unique_ptr<Action<T>>> actions)
			: actions(actions){
			alignMaxHealth();
		}

		float update(const float delta, T* elem) override{
			for(auto& action : this->actions){
				action->update(delta, elem);
			}

			return Action<T>::update(delta, elem);
		}
	};

	template <typename T>
	struct DelayAction : Action<T>{
		using Action<T>::scale;
		using Action<T>::getInterp;

		explicit DelayAction(const float lifetime) : Action<T>(lifetime){}

		float update(const float delta, T* elem) override{
			float ret = -1.0f;

			if(scale.time >= scale.lifetime)return ret;

			scale.time += delta;

			if(scale.time >= scale.lifetime){
				ret = scale.time - scale.lifetime;
				scale.time = scale.lifetime;
			}

			return ret;
		}
	};
}
