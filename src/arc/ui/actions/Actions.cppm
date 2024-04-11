//
// Created by Matrix on 2024/3/30.
//

export module UI.Action.Actions;

export import UI.Elem;
export import UI.Action;

import Graphic.Color;
import Geom.Vector2D;
import Concepts;

import std;

using namespace Graphic;

export namespace UI::Actions{
	struct ColorAction : Action<Elem>{
	protected:
		Color beginColor;

	public:
		Color endColor;

		ColorAction(const float lifetime, const Color& endColor)
			: Action<Elem>(lifetime),
			  endColor(endColor){}

		ColorAction(const float lifetime, const Color& beginColor, const Math::Interp::InterpFunc& interpFunc)
			: Action<Elem>(lifetime, interpFunc),
			  beginColor(beginColor){}

		void apply(Elem* elem, const float progress) override{
			elem->color = beginColor.createLerp(endColor, progress);
		}

		void begin(Elem* elem) override{
			beginColor = elem->color;
		}

		void end(Elem* elem) override{
			elem->color = endColor;
		}
	};

	struct AlphaAction : Action<Elem>{
	protected:
		float beginAlpha{};

	public:
		float endAlpha{};

		AlphaAction(const float lifetime, const float endAlpha, const Math::Interp::InterpFunc& interpFunc)
			: Action<Elem>(lifetime, interpFunc),
			  endAlpha(endAlpha){}

		AlphaAction(const float lifetime, const float endAlpha)
			: Action<Elem>(lifetime),
			  endAlpha(endAlpha){}

		void apply(Elem* elem, const float progress) override{
			elem->selfMaskOpacity = Math::lerp(beginAlpha, endAlpha, progress);
		}

		void begin(Elem* elem) override{
			beginAlpha = elem->selfMaskOpacity;
		}

		void end(Elem* elem) override{
			elem->selfMaskOpacity = endAlpha;
		}
	};

	struct AlphaMaskAction final : AlphaAction{
		AlphaMaskAction(const float lifetime, const float endAlpha, Math::Interp::InterpFunc& interpFunc)
			: AlphaAction{lifetime, endAlpha, interpFunc}{}

		AlphaMaskAction(const float lifetime, const float endAlpha)
			: AlphaAction{lifetime, endAlpha}{}

		void apply(Elem* elem, const float progress) override{

			elem->maskOpacity = Math::lerp(beginAlpha, endAlpha, progress);
		}

		void begin(Elem* elem) override{
			beginAlpha = elem->maskOpacity;
		}

		void end(Elem* elem) override{
			elem->maskOpacity = endAlpha;
		}
	};

	struct RemoveAction : Action<Elem>{
		RemoveAction() = default;

		void begin(Elem* elem) override;
	};

	template <typename T, Concepts::Invokable<void(T*)> Func>

	struct RunnableAction : Action<T>{
		Func func{};

		explicit RunnableAction(const Func& func) : func{func}{}
		explicit RunnableAction(Func&& func) : func{std::forward<Func>(func)}{}

		RunnableAction(const RunnableAction& other) = delete;

		RunnableAction(RunnableAction&& other) noexcept = delete;

		RunnableAction& operator=(const RunnableAction& other) = delete;

		RunnableAction& operator=(RunnableAction&& other) noexcept = delete;

		void end(T* elem) override{
			func(elem);
		}
	};

	template <typename T, Concepts::Invokable<void(T*)> Func>
	RunnableAction(Func&) -> RunnableAction<T, Func>;
}
