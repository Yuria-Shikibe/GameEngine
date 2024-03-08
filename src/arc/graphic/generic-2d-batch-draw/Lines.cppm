module;

export module Graphic.Draw.Lines;

export import Graphic.Draw;
export import Geom.Vector2D;

import <span>;
import <vector>;

namespace Graphic::Draw::Line {
	std::vector<Geom::Vec2> pointBuffer{};

	bool buildingLine = false;
	bool closedLine = false;

	inline Color beginColor{};
	inline Color endColor{};
}

export namespace Graphic::Draw::Line {
	void setLerpColor(const Color& begin = Draw::contextColor, const Color& end = Draw::contextColor){
		beginColor = begin;
		endColor = end;
	}

	//TODO support a color seq maybe in the future
	void beginLineVert(const bool closed = false){
		pointBuffer.clear();
		buildingLine = true;
		closedLine = closed;
	}

	void push(const Geom::Vec2 vec2) {
		pointBuffer.push_back(vec2);
	}

	void push(const std::span<Geom::Vec2> verts) {
		for(const auto vec2 : verts){
			pointBuffer.push_back(vec2);
		}
	}

	template <Concepts::InvokeNullable<void(Geom::Vec2, Graphic::Color)> Func = nullptr_t>
	void endLineVert(Func&& func = nullptr) {
		buildingLine  = false;
		if(pointBuffer.empty())return;

		if(closedLine) {
			pointBuffer.emplace_back(pointBuffer.front());
		}

		const auto size = pointBuffer.size() - 1;
		const auto sizeF = static_cast<float>(size);

		Color lineBeginLerp = beginColor;
		Color lineEndLerp = endColor;

		const bool enableLerp = beginColor != endColor;
		int currentIndex = 0;
		for(; currentIndex < size; ++currentIndex) {
			if(enableLerp){
				lineBeginLerp.lerp(static_cast<float>(currentIndex) / sizeF, beginColor, endColor);
				lineEndLerp.lerp(static_cast<float>(currentIndex + 1) / sizeF, beginColor, endColor);
			}

			Draw::line(pointBuffer[currentIndex], pointBuffer[currentIndex + 1], lineBeginLerp, lineEndLerp);
			if constexpr (!std::is_same_v<Func, nullptr_t>){
				func(pointBuffer[currentIndex], lineBeginLerp);
			}
		}

		if constexpr (!std::is_same_v<Func, nullptr_t>){
			func(pointBuffer[currentIndex], lineEndLerp);
		}
	}

	template <typename ...Args>
		requires (std::is_same_v<Geom::Vec2, Args> && ...)
	void outline(const bool close, Args... args) {
		beginLineVert(close);
		(::Graphic::Draw::Line::push(args), ...);
		endLineVert();
	}


	void outline(const std::span<Geom::Vec2> verts, const bool close = false) {
		beginLineVert(close);
		for(const auto p : verts){
			push(p);
		}
		endLineVert();
	}
}