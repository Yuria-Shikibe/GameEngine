//
// Created by Matrix on 2024/3/30.
//

export module UI.Action.Actions;

export import UI.Elem;
export import UI.Action;

import Graphic.Color;
import Geom.Vector2D;

export namespace UI::Actions{
	using namespace Graphic;
	struct ColorAction : Action<Elem>{
	protected:
		Color beginColor;

	public:
		Color endColor;

		ColorAction(const float lifetime, const Color& endColor)
			: Action<Elem>(lifetime),
			  endColor(endColor){}

		ColorAction(const float lifetime, const Color& beginColor, const Math::Interp::InterpFunc* interpFunc)
			: Action<Elem>(lifetime, interpFunc),
			  beginColor(beginColor){}

		void apply(Elem* elem, float progress) override{
			elem->color = beginColor.createLerp(endColor, progress);
		}

		void begin(Elem* elem) override{
			beginColor = elem->color;
		}

		void end(Elem* elem) override{
			elem->color = endColor;
		}
	};

	using namespace Graphic;
	struct AlphaAction : Action<Elem>{
	protected:
		float beginAlpha{};

	public:
		float endAlpha{};

		AlphaAction(const float lifetime, const float endAlpha, const Math::Interp::InterpFunc* interpFunc)
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
}
