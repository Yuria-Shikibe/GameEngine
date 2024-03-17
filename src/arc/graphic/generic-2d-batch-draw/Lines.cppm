module;

export module Graphic.Draw.Lines;

export import Graphic.Draw;
export import Geom.Vector2D;

import Geom.Shape.Rect_Orthogonal;

import Math;

import std;

namespace Graphic::Draw::Line {
	std::vector<Geom::Vec2> pointBuffer{};

	bool buildingLine = false;
	bool closedLine = false;

	inline Color beginColor{};
	inline Color endColor{};

	Geom::Vec2 vec2_0{};
	Geom::Vec2 vec2_1{};
	Geom::Vec2 vec2_2{};
	Geom::Vec2 vec2_3{};
	Geom::Vec2 vec2_4{};
	Geom::Vec2 vec2_5{};
	Geom::Vec2 vec2_6{};
}

export namespace Graphic::Draw::Line {
	using namespace GL;
	using namespace Geom;
	void line(const TextureRegion* region, const float x, const float y, const float x2, const float y2,
	          const Color& c1 = contextColor, const Color& c2 = contextColor, const bool cap = true) {
		const float h_stroke = contextStroke / 2.0f;
		const float len      = Math::len(x2 - x, y2 - y);
		const float diff_x   = (x2 - x) / len * h_stroke;
		const float diff_y   = (y2 - y) / len * h_stroke;

		if(cap) {
			quad(
				region,
				Vec2{x - diff_x - diff_y, y - diff_y + diff_x}, c1,
				Vec2{x - diff_x + diff_y, y - diff_y - diff_x}, c1,
				Vec2{x2 + diff_x + diff_y, y2 + diff_y - diff_x}, c2,
				Vec2{x2 + diff_x - diff_y, y2 + diff_y + diff_x}, c2
			);
		} else {
			quad(
				region,
				Vec2{x - diff_y, y + diff_x}, c1,
				Vec2{x + diff_y, y - diff_x}, c1,
				Vec2{x2 + diff_y, y2 - diff_x}, c2,
				Vec2{x2 - diff_y, y2 + diff_x}, c2
			);
		}
	}

	void line(const float x, const float y, const float x2, const float y2, const bool cap = true) {
		line(defaultTexture, x, y, x2, y2, contextColor, contextColor, cap);
	}

	void line(const Vec2 v1, const Vec2 v2, const Color& c1 = contextColor, const Color& c2 = contextColor, const bool cap = true) {
		line(defaultTexture, v1.x, v1.y, v2.x, v2.y, c1, c2, cap);
	}

	inline void setLineStroke(const float s) {
		contextStroke = s;
	}

	inline void lineAngleCenter(const float x, const float y, const float angle, const float length,
	                            const bool cap = true) {
		vec2_0.setPolar(angle, length * 0.5f);

		line(contextTexture, x - vec2_0.x, y - vec2_0.y, x + vec2_0.x, y + vec2_0.y, contextColor, contextColor, cap);
	}

	inline void lineAngle(const float x, const float y, const float angle, const float length, const bool cap = true) {
		vec2_0.setPolar(angle, length);

		line(contextTexture, x, y, x + vec2_0.x, y + vec2_0.y, contextColor, contextColor, cap);
	}

	inline void lineAngle(const float x, const float y, const float angle, const float length, const float offset) {
		vec2_0.setPolar(angle, 1.0f);

		line(contextTexture, x + vec2_0.x * offset, y + vec2_0.y * offset, x + vec2_0.x * (length + offset),
		     y + vec2_0.y * (length + offset));
	}

	void rect(const float srcx, const float srcy, const float width, const float height, const bool cap = true) {
		line(defaultTexture, srcx, srcy, srcx, srcy + height - contextStroke, contextColor, contextColor, cap);
		line(defaultTexture, srcx, srcy + height, srcx + width - contextStroke, srcy + height, contextColor, contextColor, cap);
		line(defaultTexture, srcx + width, srcy + height, srcx + width, srcy + contextStroke, contextColor, contextColor, cap);
		line(defaultTexture, srcx + width, srcy, srcx + contextStroke, srcy, contextColor, contextColor, cap);
	}

	void rect(const Geom::Shape::OrthoRectFloat& rect, const bool cap = true, const Vec2& offset = Geom::ZERO){
		Line::rect(rect.getSrcX() + offset.getX(), rect.getSrcY() + offset.getY(), rect.getWidth(), rect.getHeight(),
				 cap);
	}

	void square(const float x, const float y, const float radius, float ang) {
		ang += 45.000f;
		const float dst = contextStroke / Math::SQRT2;

		vec2_0.setPolar(ang, 1);

		vec2_1.set(vec2_0);
		vec2_2.set(vec2_0);

		vec2_1.scl(radius - dst);
		vec2_2.scl(radius + dst);

		for(int i = 0; i < 4; ++i) {
			vec2_0.rotateRT();

			vec2_3.set(vec2_0).scl(radius - dst);
			vec2_4.set(vec2_0).scl(radius + dst);

			quad(vec2_1.x + x, vec2_1.y + y, vec2_2.x + x, vec2_2.y + y, vec2_4.x + x, vec2_4.y + y, vec2_3.x + x,
			     vec2_3.y + y);

			vec2_1.set(vec2_3);
			vec2_2.set(vec2_4);
		}
	}

	void poly(const float x, const float y, const int sides, const float radius, const float angle) {
		const float space  = 360.0f / static_cast<float>(sides);
		const float h_step = contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
		const float r1     = radius - h_step;
		const float r2     = radius + h_step;

		for(int i = 0; i < sides; i++) {
			const float a    = space * static_cast<float>(i) + angle;
			const float cos1 = Math::cosDeg(a);
			const float sin1 = Math::sinDeg(a);
			const float cos2 = Math::cosDeg(a + space);
			const float sin2 = Math::sinDeg(a + space);
			quad(
				x + r1 * cos1, y + r1 * sin1,
				x + r1 * cos2, y + r1 * sin2,
				x + r2 * cos2, y + r2 * sin2,
				x + r2 * cos1, y + r2 * sin1
			);
		}
	}

	void poly(const float x, const float y, const int sides, const float radius, const float angle, const float ratio,
	          const auto&... args) {
		constexpr auto size = sizeof...(args);
		const auto colors = std::make_tuple(args...);

		const auto fSides = static_cast<float>(sides);

		const float space  = 360.0f / fSides;
		const float h_step = contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
		const float r1     = radius - h_step;
		const float r2     = radius + h_step;

		float currentRatio = 0;

		float currentAng = angle;
		float sin1       = Math::sinDeg(currentAng);
		float cos1       = Math::cosDeg(currentAng);
		float sin2, cos2;

		float progress   = 0;
		Color lerpColor1 = std::get<0>(colors);
		Color lerpColor2 =std::get<size - 1>(colors);

		for(; progress < fSides * ratio - 1.0f; progress += 1.0f) {
			// NOLINT(cert-flp30-c)
			currentAng = angle + (progress + 1.0f) * space;

			sin2 = Math::sinDeg(currentAng);
			cos2 = Math::cosDeg(currentAng);

			currentRatio = progress / fSides;

			lerpColor2.lerp(currentRatio, args...);

			quad(defaultTexture,
			     cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
			     cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
			     cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
			     cos2 * r1 + x, sin2 * r1 + y, lerpColor2
			);

			lerpColor1.set(lerpColor2);

			sin1 = sin2;
			cos1 = cos2;
		}

		currentRatio            = ratio;
		const float remainRatio = currentRatio * fSides - progress;

		currentAng = angle + (progress + 1.0f) * space;

		sin2 = Math::lerp(sin1, Math::sinDeg(currentAng), remainRatio);
		cos2 = Math::lerp(cos1, Math::cosDeg(currentAng), remainRatio);

		lerpColor2.lerp(progress / fSides, args...).lerp(lerpColor1, 1.0f - remainRatio);

		quad(defaultTexture,
		     cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
		     cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
		     cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
		     cos2 * r1 + x, sin2 * r1 + y, lerpColor2
		);
	}

	void poly(const float x, const float y, const int sides, const float radius, const float angle, const float ratio,
			  const std::span<const Color>& colorGroup) {
		const auto size = colorGroup.size();

		const auto fSides = static_cast<float>(sides);

		const float space  = 360.0f / fSides;
		const float h_step = contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
		const float r1     = radius - h_step;
		const float r2     = radius + h_step;

		float currentRatio = 0;

		float currentAng = angle;
		float sin1       = Math::sinDeg(currentAng);
		float cos1       = Math::cosDeg(currentAng);
		float sin2, cos2;

		float progress   = 0;
		Color lerpColor1 = colorGroup[0x000000];
		Color lerpColor2 = colorGroup[size - 1];

		for(; progress < fSides * ratio - 1.0f; progress += 1.0f) {
			// NOLINT(cert-flp30-c)
			currentAng = angle + (progress + 1.0f) * space;

			sin2 = Math::sinDeg(currentAng);
			cos2 = Math::cosDeg(currentAng);

			currentRatio = progress / fSides;

			lerpColor2.lerp(currentRatio, colorGroup);

			quad(defaultTexture,
				 cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
				 cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
				 cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
				 cos2 * r1 + x, sin2 * r1 + y, lerpColor2
			);

			lerpColor1.set(lerpColor2);

			sin1 = sin2;
			cos1 = cos2;
		}

		currentRatio            = ratio;
		const float remainRatio = currentRatio * fSides - progress;

		currentAng = angle + (progress + 1.0f) * space;

		sin2 = Math::lerp(sin1, Math::sinDeg(currentAng), remainRatio);
		cos2 = Math::lerp(cos1, Math::cosDeg(currentAng), remainRatio);

		lerpColor2.lerp(progress / fSides, colorGroup).lerp(lerpColor1, 1.0f - remainRatio);

		quad(defaultTexture,
			 cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
			 cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
			 cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
			 cos2 * r1 + x, sin2 * r1 + y, lerpColor2
		);
	}

	void circle(const float x, const float y, const float radius){
		poly(x, y, Draw::getCircleVerts(radius), radius, 0);
	}

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

	template <Concepts::InvokeNullable<void(Geom::Vec2, Graphic::Color)> Func = std::nullptr_t>
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

			line(pointBuffer[currentIndex], pointBuffer[currentIndex + 1], lineBeginLerp, lineEndLerp);
			if constexpr (!std::is_same_v<Func, std::nullptr_t>){
				func(pointBuffer[currentIndex], lineBeginLerp);
			}
		}

		if constexpr (!std::is_same_v<Func, std::nullptr_t>){
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