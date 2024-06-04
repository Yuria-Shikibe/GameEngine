//
// Created by Matrix on 2024/5/26.
//

export module Graphic.Draw:GeomDrawBase;

import ext.Concepts;

export import Core.BatchGroup;
export import Graphic.Color;
export import GL.Texture;
export import GL.Texture.TextureRegion;
export import Math;

import std;

import :GlobalData;

export namespace Graphic{
	struct HasBatch{
		[[noreturn]] static Core::Batch& getBatch(){
			std::unreachable();
		}
	};

	template <typename Identity = HasBatch>
	struct VertPostBase{
		using ImplTy = Identity;
		inline static const GL::TextureRegion* contextTexture = nullptr;
		inline static const GL::TextureRegion* defaultTexture = nullptr;

		//TODO provide a alpha mask override?
		inline static Color contextColor = Colors::WHITE;
		inline static Color contextMixColor = Colors::CLEAR;

		static Core::Batch& getBindedBatch(){
			return ImplTy::getBatch();
		}

		static constexpr float circleVertPrecision{8};

		static constexpr int getCircleVerts(const float radius){
			return Math::max(Math::ceil(radius * Math::PI / circleVertPrecision), 12);
		}

		static const GL::TextureRegion& getContextTexture() noexcept{
			return *contextTexture;
		}

		static const GL::TextureRegion& getDefaultTexture() noexcept{
			return *defaultTexture;
		}

		template <bool applyAlpha = true>
		static void color(const Color color = Colors::WHITE) noexcept{
			if constexpr(applyAlpha){
				contextColor = color;
			} else{
				contextColor.set(color.r, color.g, color.b);
			}
		}

		static void color(const Color color, const float alpha) noexcept{
			contextColor.set(color.r, color.g, color.b, alpha);
		}

		static void tint(const Color color = Colors::WHITE, const float alpha = 0.0f) noexcept{
			contextColor.set(color.r, color.g, color.b, alpha);
		}

		template <bool applyAlpha = true>
		static void mixColor(const Color color = Colors::CLEAR) noexcept{
			if constexpr(applyAlpha){
				contextMixColor = color;
			} else{
				contextMixColor.set(color.r, color.g, color.b);
			}
		}

		static void alpha(const float a = 1.0f) noexcept{ contextColor.setA(a); }

		static void mixAlpha(const float a) noexcept{ contextMixColor.setA(a); }

		static void mixMulAlpha(const float a) noexcept{ contextMixColor.mulA(a); }

		static void color(const Color c1, const Color c2, const float t) noexcept{ contextColor.lerp(t, c1, c2); }

		template <bool setContext = true>
		static void setDefTexture(const GL::TextureRegion* texture) noexcept{
			defaultTexture = texture;

			if constexpr(setContext){
				contextTexture = texture;
			}
		}

		static void setTexture(const GL::TextureRegion* texture = defaultTexture) noexcept{
			contextTexture = texture;
		}

		static void reset() noexcept{
			color();
			mixColor();
			setTexture();
		}

		static void vert(
			const GL::Texture* texture,
			float x1, float y1, float u1, float v1, Color c1, Color cm1,
			float x2, float y2, float u2, float v2, Color c2, Color cm2,
			float x3, float y3, float u3, float v3, Color c3, Color cm3,
			float x4, float y4, float u4, float v4, Color c4, Color cm4
		){
			std::unreachable();
		};

		static void vert_monochromeMix(
			const GL::Texture* texture, Color cm,
			float x1, float y1, float u1, float v1, Color c1,
			float x2, float y2, float u2, float v2, Color c2,
			float x3, float y3, float u3, float v3, Color c3,
			float x4, float y4, float u4, float v4, Color c4
		){
			std::unreachable();
		};

		static void vert_monochromeAll(
			const GL::Texture* texture, Color cm, Color c,
			float x1, float y1, float u1, float v1,
			float x2, float y2, float u2, float v2,
			float x3, float y3, float u3, float v3,
			float x4, float y4, float u4, float v4
		){
			std::unreachable();
		};
	};

	using DefHolerBase = VertPostBase<HasBatch>;

	// using VERT_POST_PROV_Ty = DefHolerBase;
	template <typename VERT_POST_PROV_Ty = DefHolerBase>
	struct DrawBase : VERT_POST_PROV_Ty{
		//Uses ptr/template arg to create a context data copy
		using ProvTy = VERT_POST_PROV_Ty;

		using ProvTy::defaultTexture;
		using ProvTy::contextTexture;
		using ProvTy::contextColor;
		using ProvTy::contextMixColor;

		struct Fill{
			static void quad(const GL::TextureRegion& region,
				const float x1, const float y1, const Color c1,
				const float x2, const float y2, const Color c2,
				const float x3, const float y3, const Color c3,
				const float x4, const float y4, const Color c4
			){
				ProvTy::vert_monochromeMix(
					region.data, ProvTy::contextMixColor,
					x1, y1, region.v00.x, region.v00.y, c1,
					x2, y2, region.v10.x, region.v10.y, c2,
					x3, y3, region.v11.x, region.v11.y, c3,
					x4, y4, region.v01.x, region.v01.y, c4
				);
			}


			static void quad(const GL::TextureRegion& region,
				const Geom::Vec2 v0, const Color c1,
				const Geom::Vec2 v1, const Color c2,
				const Geom::Vec2 v2, const Color c3,
				const Geom::Vec2 v3, const Color c4
			){
				ProvTy::vert_monochromeMix(
					region.data, ProvTy::contextMixColor,
					v0.x, v0.y, region.v00.x, region.v00.y, c1,
					v1.x, v1.y, region.v10.x, region.v10.y, c2,
					v2.x, v2.y, region.v11.x, region.v11.y, c3,
					v3.x, v3.y, region.v01.x, region.v01.y, c4
				);
			}

			static void quad(const GL::TextureRegion& region,
				Concepts::Pos<float> auto&& v0, Concepts::Pos<float> auto&& v1,
				Concepts::Pos<float> auto&& v2, Concepts::Pos<float> auto&& v3
			){
				ProvTy::vert_monochromeAll(
					region.data, ProvTy::contextColor, ProvTy::contextMixColor,
					v0.getX(), v0.getY(), region.v00.x, region.v00.y,
					v1.getX(), v1.getY(), region.v10.x, region.v10.y,
					v2.getX(), v2.getY(), region.v11.x, region.v11.y,
					v3.getX(), v3.getY(), region.v01.x, region.v01.y
				);
			}

			static void quad(
				Concepts::Pos<float> auto&& v0, Concepts::Pos<float> auto&& v1,
				Concepts::Pos<float> auto&& v2, Concepts::Pos<float> auto&& v3
			){
				ProvTy::vert_monochromeAll(
					ProvTy::contextTexture->data, ProvTy::contextColor, ProvTy::contextMixColor,
					v0.getX(), v0.getY(), ProvTy::contextTexture->v00.x, ProvTy::contextTexture->v00.y,
					v1.getX(), v1.getY(), ProvTy::contextTexture->v10.x, ProvTy::contextTexture->v10.y,
					v2.getX(), v2.getY(), ProvTy::contextTexture->v11.x, ProvTy::contextTexture->v11.y,
					v3.getX(), v3.getY(), ProvTy::contextTexture->v01.x, ProvTy::contextTexture->v01.y
				);
			}

			static void quad(const GL::TextureRegion& region,
				const float x1, const float y1,
				const float x2, const float y2,
				const float x3, const float y3,
				const float x4, const float y4
			){
				ProvTy::vert_monochromeAll(
					region.data, ProvTy::contextColor, ProvTy::contextMixColor,
					x1, y1, region.v00.x, region.v00.y,
					x2, y2, region.v10.x, region.v10.y,
					x3, y3, region.v11.x, region.v11.y,
					x4, y4, region.v01.x, region.v01.y
				);
			}



			static void rectOrtho(const GL::TextureRegion& region,
				const float x, const float y,
				const float w, const float h
			){
				ProvTy::vert_monochromeAll(
					region.data, ProvTy::contextColor, ProvTy::contextMixColor,
					x, y, region.v00.x, region.v00.y,
					x + w, y, region.v10.x, region.v10.y,
					x + w, y + h, region.v11.x, region.v11.y,
					x, y + h, region.v01.x, region.v01.y
				);
			}

			static void rectOrtho(const GL::TextureRegion& region, const Geom::OrthoRectFloat rect){
				Fill::rectOrtho(region, rect.getSrcX(), rect.getSrcY(), rect.getWidth(), rect.getHeight());
			}

			static void rectOrtho(const GL::TextureRegion& region,
				const Geom::Vec2 vert00,
				const Geom::Vec2 vert11
			){
				Fill::rectOrtho(region, Geom::OrthoRectFloat{vert00, vert11});
			}


			static void rectPoint(const Geom::Vec2 pos, const float size){
				Fill::rectOrtho(ProvTy::contextTexture, pos.x - size * 0.5f, pos.y - size * 0.5f, size, size);
			}


			static void rect(const GL::TextureRegion& region,
				const float x, const float y, const float ang,
				const float width, const float height
			){
				const float sin = Math::sinDeg(ang);
				const float cos = Math::cosDeg(ang);
				const float w1 = cos * width * 0.5f;
				const float h1 = sin * width * 0.5f;

				const float w2 = -sin * height * 0.5f;
				const float h2 = cos * height * 0.5f;
				ProvTy::vert_monochromeAll(
					region.data, ProvTy::contextColor, ProvTy::contextMixColor,
					x - w1 - w2, y - h1 - h2, region.v00.x, region.v00.y,
					x + w1 - w2, y + h1 - h2, region.v10.x, region.v10.y,
					x + w1 + w2, y + h1 + h2, region.v11.x, region.v11.y,
					x - w1 + w2, y - h1 + h2, region.v01.x, region.v01.y
				);
			}



			static void rect(const GL::TextureRegion& region,
				const float x, const float y,
				const float ang = 0, const Geom::Vec2 scl = Geom::norBaseVec2<float>
			){
				Fill::rect(region, x, y, ang, region.getWidth() * scl.x, region.getHeight() * scl.y);
			}


			static void rect(const GL::TextureRegion& region, Concepts::Derived<Geom::Transform> auto trans,
				const Geom::Vec2 scl = Geom::norBaseVec2<float>){
				Fill::rect(region, trans.vec.x, trans.vec.y, trans.rot, scl);
			}


			static void rectOrtho(const float x, const float y, const float w, const float h){
				Fill::rectOrtho(*ProvTy::contextTexture, x, y, w, h);
			}


			static void quad(const GL::TextureRegion& region, const Geom::OrthoRectFloat rect, const float x = 0,
				const float y = 0){
				ProvTy::vert_monochromeAll(
					region.data, ProvTy::contextColor, ProvTy::contextMixColor,
					rect.getSrcX() + x, rect.getSrcY() + y, region.v00.x, region.v00.y,
					rect.getSrcX() + x, rect.getEndY() + y, region.v10.x, region.v10.y,
					rect.getEndX() + x, rect.getEndY() + y, region.v11.x, region.v11.y,
					rect.getEndX() + x, rect.getSrcY() + y, region.v01.x, region.v01.y
				);
			}


			static void quad(
				const float x1, const float y1,
				const float x2, const float y2,
				const float x3, const float y3,
				const float x4, const float y4
			){
				ProvTy::vert_monochromeAll(
					ProvTy::contextTexture->data, ProvTy::contextColor, ProvTy::contextMixColor,
					x1, y1, ProvTy::contextTexture->v00.x, ProvTy::contextTexture->v00.y,
					x2, y2, ProvTy::contextTexture->v10.x, ProvTy::contextTexture->v10.y,
					x3, y3, ProvTy::contextTexture->v11.x, ProvTy::contextTexture->v11.y,
					x4, y4, ProvTy::contextTexture->v01.x, ProvTy::contextTexture->v01.y
				);
			}


			static void square(const float x, const float y, const float radius, const float ang){
				vec2_0.setPolar(ang + 45.0f, radius / Math::SQRT2);

				vec2_1.set(x, y).add(vec2_0);
				vec2_2.set(x, y).add(vec2_0.rotateRT());

				vec2_3.set(x, y).add(vec2_0.rotateRT());
				vec2_4.set(x, y).add(vec2_0.rotateRT());

				Fill::quad(*ProvTy::contextTexture, vec2_1, vec2_2, vec2_3, vec2_4);
			}


			static void poly(const float x, const float y, const int sides, const float radius, const float angle,
				const Color inner = ProvTy::contextColor, const Color exter = ProvTy::contextColor){
				const float space = 360.0f / static_cast<float>(sides);

				for(int i = 0; i < sides; i++){
					const float a = space * static_cast<float>(i) + angle;
					const float cos1 = Math::cosDeg(a);
					const float sin1 = Math::sinDeg(a);
					const float cos2 = Math::cosDeg(a + space);
					const float sin2 = Math::sinDeg(a + space);
					Fill::quad(
						*ProvTy::contextTexture,
						x, y, inner,
						x, y, inner,
						x + radius * cos2, y + radius * sin2, exter,
						x + radius * cos1, y + radius * sin1, exter
					);
				}
			}


			static void circle(const float x, const float y, const float radius,
				const Color inner = ProvTy::contextColor, const Color exter = ProvTy::contextColor){
				Fill::poly(x, y, ProvTy::getCircleVerts(radius), radius, 0, inner, exter);
			}
		};

		struct Line{
		private:
			inline static std::vector<Geom::Vec2> pointBuffer{};

			inline static bool buildingLine = false;
			inline static bool closedLine = false;

			inline static Color beginColor{};
			inline static Color endColor{};

		public:
			inline static float contextStroke = 1.0f;

			static void line(const GL::TextureRegion& region, const float x, const float y, const float x2,
				const float y2,
				const Color c1 = ProvTy::contextColor, const Color c2 = ProvTy::contextColor, const bool cap = true){
				const float h_stroke = Line::contextStroke / 2.0f;
				const float len = Math::len(x2 - x, y2 - y);
				const float diff_x = (x2 - x) / len * h_stroke;
				const float diff_y = (y2 - y) / len * h_stroke;

				if(cap){
					Fill::quad(
						region,
						x - diff_x - diff_y, y - diff_y + diff_x, c1,
						x - diff_x + diff_y, y - diff_y - diff_x, c1,
						x2 + diff_x + diff_y, y2 + diff_y - diff_x, c2,
						x2 + diff_x - diff_y, y2 + diff_y + diff_x, c2
					);
				} else{
					Fill::quad(
						region,
						x - diff_y, y + diff_x, c1,
						x + diff_y, y - diff_x, c1,
						x2 + diff_y, y2 - diff_x, c2,
						x2 - diff_y, y2 + diff_x, c2
					);
				}
			}


			static void line(const float x, const float y, const float x2, const float y2, const bool cap = true){
				Line::line(*ProvTy::contextTexture, x, y, x2, y2, ProvTy::contextColor, ProvTy::contextColor, cap);
			}


			static void line(const Geom::Vec2 v1, const Geom::Vec2 v2, const Color c1 = ProvTy::contextColor,
				const Color c2 = ProvTy::contextColor,
				const bool cap = true){
				Line::line(*ProvTy::contextTexture, v1.x, v1.y, v2.x, v2.y, c1, c2, cap);
			}


			static void setLineStroke(const float s){
				Line::contextStroke = s;
			}


			static void lineAngleCenter(const float x, const float y, const float angle, const float length,
				const bool cap = true){
				vec2_0.setPolar(angle, length * 0.5f);

				Line::line(*ProvTy::contextTexture, x - vec2_0.x, y - vec2_0.y, x + vec2_0.x, y + vec2_0.y,
					ProvTy::contextColor,
					ProvTy::contextColor,
					cap);
			}


			static void lineAngle(const float x, const float y, const float angle, const float length,
				const bool cap = true){
				vec2_0.setPolar(angle, length);

				Line::line(*ProvTy::contextTexture, x, y, x + vec2_0.x, y + vec2_0.y, ProvTy::contextColor,
					ProvTy::contextColor, cap);
			}


			static void lineAngle(const Geom::Transform trans, const float length,
				const bool cap = true){
				lineAngle(trans.vec.x, trans.vec.y, trans.rot, length, cap);
			}


			static void lineAngle(const float x, const float y, const float angle, const float length,
				const float offset){
				vec2_0.setPolar(angle, 1.0f);

				Line::line(*ProvTy::contextTexture, x + vec2_0.x * offset, y + vec2_0.y * offset,
					x + vec2_0.x * (length + offset),
					y + vec2_0.y * (length + offset));
			}


			static void quad(const Geom::QuadBox& box, const bool cap = true){
				for(int i = 0; i < 4; ++i){
					Line::line(box[i], box[(i + 1) % 4], ProvTy::contextColor, ProvTy::contextColor, cap);
				}
			}


			static void rectOrtho(const float srcx, const float srcy, const float width, const float height,
				const bool cap = true){
				Line::line(*ProvTy::contextTexture, srcx, srcy, srcx, srcy + height - Line::contextStroke,
					ProvTy::contextColor,
					ProvTy::contextColor, cap);
				Line::line(*ProvTy::contextTexture, srcx, srcy + height, srcx + width - Line::contextStroke,
					srcy + height,
					ProvTy::contextColor,
					ProvTy::contextColor, cap);
				Line::line(*ProvTy::contextTexture, srcx + width, srcy + height, srcx + width,
					srcy + Line::contextStroke,
					ProvTy::contextColor,
					ProvTy::contextColor, cap);
				Line::line(*ProvTy::contextTexture, srcx + width, srcy, srcx + Line::contextStroke, srcy,
					ProvTy::contextColor,
					ProvTy::contextColor, cap);
			}


			static void rectOrtho(const Geom::OrthoRectFloat& rect, const bool cap = true,
				const Geom::Vec2 offset = Geom::ZERO){
				Line::rectOrtho(rect.getSrcX() + offset.getX(), rect.getSrcY() + offset.getY(), rect.getWidth(),
					rect.getHeight(),
					cap);
			}


			static void square(const float x, const float y, const float radius, float ang){
				ang += 45.000f;
				const float dst = Line::contextStroke / Math::SQRT2;

				vec2_0.setPolar(ang, 1);

				vec2_1.set(vec2_0);
				vec2_2.set(vec2_0);

				vec2_1.scl(radius - dst);
				vec2_2.scl(radius + dst);

				for(int i = 0; i < 4; ++i){
					vec2_0.rotateRT();

					vec2_3.set(vec2_0).scl(radius - dst);
					vec2_4.set(vec2_0).scl(radius + dst);

					Fill::quad(vec2_1.x + x, vec2_1.y + y, vec2_2.x + x, vec2_2.y + y, vec2_4.x + x, vec2_4.y + y,
						vec2_3.x + x,
						vec2_3.y + y);

					vec2_1.set(vec2_3);
					vec2_2.set(vec2_4);
				}
			}


			static void poly(const float x, const float y, const int sides, const float radius, const float angle){
				const float space = 360.0f / static_cast<float>(sides);
				const float h_step = Line::contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
				const float r1 = radius - h_step;
				const float r2 = radius + h_step;

				for(int i = 0; i < sides; i++){
					const float a = space * static_cast<float>(i) + angle;
					const float cos1 = Math::cosDeg(a);
					const float sin1 = Math::sinDeg(a);
					const float cos2 = Math::cosDeg(a + space);
					const float sin2 = Math::sinDeg(a + space);
					Fill::quad(
						x + r1 * cos1, y + r1 * sin1,
						x + r1 * cos2, y + r1 * sin2,
						x + r2 * cos2, y + r2 * sin2,
						x + r2 * cos1, y + r2 * sin1
					);
				}
			}


			static void poly(const float x, const float y, const int sides, const float radius, const float angle,
				const float ratio,
				const auto&... args){
				constexpr auto size = sizeof...(args);
				const auto colors = std::make_tuple(args...);

				const auto fSides = static_cast<float>(sides);

				const float space = 360.0f / fSides;
				const float h_step = Line::contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
				const float r1 = radius - h_step;
				const float r2 = radius + h_step;

				float currentRatio = 0;

				float currentAng = angle;
				float sin1 = Math::sinDeg(currentAng);
				float cos1 = Math::cosDeg(currentAng);
				float sin2, cos2;

				float progress = 0;
				Color lerpColor1 = std::get<0>(colors);
				Color lerpColor2 = std::get<size - 1>(colors);

				for(; progress < fSides * ratio - 1.0f; progress += 1.0f){
					// NOLINT(cert-flp30-c)
					currentAng = angle + (progress + 1.0f) * space;

					sin2 = Math::sinDeg(currentAng);
					cos2 = Math::cosDeg(currentAng);

					currentRatio = progress / fSides;

					lerpColor2.lerp(currentRatio, args...);

					Fill::quad(*ProvTy::contextTexture,
						cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
						cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
						cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
						cos2 * r1 + x, sin2 * r1 + y, lerpColor2
					);

					lerpColor1.set(lerpColor2);

					sin1 = sin2;
					cos1 = cos2;
				}

				currentRatio = ratio;
				const float remainRatio = currentRatio * fSides - progress;

				currentAng = angle + (progress + 1.0f) * space;

				sin2 = Math::lerp(sin1, Math::sinDeg(currentAng), remainRatio);
				cos2 = Math::lerp(cos1, Math::cosDeg(currentAng), remainRatio);

				lerpColor2.lerp(progress / fSides, args...).lerp(lerpColor1, 1.0f - remainRatio);

				Fill::quad(*ProvTy::contextTexture,
					cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
					cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
					cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
					cos2 * r1 + x, sin2 * r1 + y, lerpColor2
				);
			}


			static void poly(const float x, const float y, const int sides, const float radius, const float angle,
				const float ratio,
				const std::span<const Color>& colorGroup){
				const auto size = colorGroup.size();

				const auto fSides = static_cast<float>(sides);

				const float space = 360.0f / fSides;
				const float h_step = Line::contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
				const float r1 = radius - h_step;
				const float r2 = radius + h_step;

				float currentRatio;

				float currentAng = angle;
				float sin1 = Math::sinDeg(currentAng);
				float cos1 = Math::cosDeg(currentAng);
				float sin2, cos2;

				float progress = 0;
				Color lerpColor1 = colorGroup[0x000000];
				Color lerpColor2 = colorGroup[size - 1];

				for(; progress < fSides * ratio - 1.0f; progress += 1.0f){
					// NOLINT(cert-flp30-c)
					currentAng = angle + (progress + 1.0f) * space;

					sin2 = Math::sinDeg(currentAng);
					cos2 = Math::cosDeg(currentAng);

					currentRatio = progress / fSides;

					lerpColor2.lerp(currentRatio, colorGroup);

					Fill::quad(*ProvTy::contextTexture,
						cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
						cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
						cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
						cos2 * r1 + x, sin2 * r1 + y, lerpColor2
					);

					lerpColor1.set(lerpColor2);

					sin1 = sin2;
					cos1 = cos2;
				}

				currentRatio = ratio;
				const float remainRatio = currentRatio * fSides - progress;

				currentAng = angle + (progress + 1.0f) * space;

				sin2 = Math::lerp(sin1, Math::sinDeg(currentAng), remainRatio);
				cos2 = Math::lerp(cos1, Math::cosDeg(currentAng), remainRatio);

				lerpColor2.lerp(progress / fSides, colorGroup).lerp(lerpColor1, 1.0f - remainRatio);

				Fill::quad(*ProvTy::contextTexture,
					cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
					cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
					cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
					cos2 * r1 + x, sin2 * r1 + y, lerpColor2
				);
			}


			static void circle(const float x, const float y, const float radius){
				poly(x, y, ProvTy::getCircleVerts(radius), radius, 0);
			}


			static void setLerpColor(const Color begin = ProvTy::contextColor, const Color end = ProvTy::contextColor){
				beginColor = begin;
				endColor = end;
			}

			//TODO support a color seq maybe in the future
			static void beginLineVert(const bool closed = false){
				pointBuffer.clear();
				buildingLine = true;
				closedLine = closed;
			}

			static void push(const Geom::Vec2 vec2){
				pointBuffer.push_back(vec2);
			}

			static void push(const std::span<Geom::Vec2> verts){
				for(const auto vec2 : verts){
					pointBuffer.push_back(vec2);
				}
			}

			template <Concepts::InvokeNullable<void(Geom::Vec2, Graphic::Color)> Func = std::nullptr_t>
			static void endLineVert(Func&& func = nullptr){
				buildingLine = false;
				if(pointBuffer.empty()) return;

				if(closedLine){
					pointBuffer.emplace_back(pointBuffer.front());
				}

				const auto size = pointBuffer.size() - 1;
				const auto sizeF = static_cast<float>(size);

				Color lineBeginLerp = beginColor;
				Color lineEndLerp = endColor;

				const bool enableLerp = beginColor != endColor;
				int currentIndex = 0;
				for(; currentIndex < size; ++currentIndex){
					if(enableLerp){
						lineBeginLerp.lerp(static_cast<float>(currentIndex) / sizeF, beginColor, endColor);
						lineEndLerp.lerp(static_cast<float>(currentIndex + 1) / sizeF, beginColor, endColor);
					}

					Line::line(pointBuffer[currentIndex], pointBuffer[currentIndex + 1], lineBeginLerp,
						lineEndLerp);
					if constexpr(!std::is_same_v<Func, std::nullptr_t>){
						func(pointBuffer[currentIndex], lineBeginLerp);
					}
				}

				if constexpr(!std::is_same_v<Func, std::nullptr_t>){
					func(pointBuffer[currentIndex], lineEndLerp);
				}
			}

			template <typename... Args>
				requires (std::is_same_v<Geom::Vec2, Args> && ...)
			static void outline(const bool close, Args... args){
				beginLineVert(close);
				(Line::push(args), ...);
				endLineVert();
			}


			static void outline(const std::span<Geom::Vec2> verts, const bool close = false){
				beginLineVert(close);
				for(const auto p : verts){
					push(p);
				}
				endLineVert();
			}
		};

		struct Fancy{
			static void select_rectOrtho(const Geom::OrthoRectFloat rect, const float cornerSize = Line::contextStroke,
				const bool align = true, const float offset = 0.5f){
				static Geom::OrthoRectFloat inner{};
				static Geom::OrthoRectFloat outer{};
				inner = rect;
				outer = rect;

				inner.expand(-offset * Line::contextStroke, -offset * Line::contextStroke);
				outer.expand((1 - offset) * Line::contextStroke, (1 - offset) * Line::contextStroke);

				if(!align){
					Fill::quad(*contextTexture,
						outer.vert_00(), inner.vert_00(),
						inner.vert_00().add(0, cornerSize), outer.vert_00().add(0, cornerSize)
					);
					Fill::quad(*contextTexture,
						outer.vert_00(), inner.vert_00(),
						inner.vert_00().add(cornerSize, 0), outer.vert_00().add(cornerSize, 0)
					);

					Fill::quad(*contextTexture,
						outer.vert_10(), inner.vert_10(),
						inner.vert_10().add(0, cornerSize), outer.vert_10().add(0, cornerSize)
					);
					Fill::quad(*contextTexture,
						outer.vert_10(), inner.vert_10(),
						inner.vert_10().add(-cornerSize, 0), outer.vert_10().add(-cornerSize, 0)
					);

					Fill::quad(*contextTexture,
						outer.vert_01(), inner.vert_01(),
						inner.vert_01().add(0, -cornerSize), outer.vert_01().add(0, -cornerSize)
					);
					Fill::quad(*contextTexture,
						outer.vert_01(), inner.vert_01(),
						inner.vert_01().add(cornerSize, 0), outer.vert_01().add(cornerSize, 0)
					);

					Fill::quad(*contextTexture,
						outer.vert_11(), inner.vert_11(),
						inner.vert_11().add(0, -cornerSize), outer.vert_11().add(0, -cornerSize)
					);
					Fill::quad(*contextTexture,
						outer.vert_11(), inner.vert_11(),
						inner.vert_11().add(-cornerSize, 0), outer.vert_11().add(-cornerSize, 0)
					);
				} else{
					Fill::quad(*contextTexture,
						outer.vert_00(), inner.vert_00(),
						Geom::Vec2{inner.vert_00().x, rect.vert_00().y + cornerSize},
						Geom::Vec2{outer.vert_00().x, rect.vert_00().y + cornerSize}
					);
					Fill::quad(*contextTexture,
						outer.vert_00(), inner.vert_00(),
						Geom::Vec2{rect.vert_00().x + cornerSize, inner.vert_00().y},
						Geom::Vec2{rect.vert_00().x + cornerSize, outer.vert_00().y}
					);

					Fill::quad(*contextTexture,
						outer.vert_10(), inner.vert_10(),
						Geom::Vec2{inner.vert_10().x, rect.vert_10().y + cornerSize},
						Geom::Vec2{outer.vert_10().x, rect.vert_10().y + cornerSize}
					);
					Fill::quad(*contextTexture,
						outer.vert_10(), inner.vert_10(),
						Geom::Vec2{rect.vert_10().x - cornerSize, inner.vert_10().y},
						Geom::Vec2{rect.vert_10().x - cornerSize, outer.vert_10().y}
					);

					Fill::quad(*contextTexture,
						outer.vert_01(), inner.vert_01(),
						Geom::Vec2{inner.vert_01().x, rect.vert_01().y - cornerSize},
						Geom::Vec2{outer.vert_01().x, rect.vert_01().y - cornerSize}
					);
					Fill::quad(*contextTexture,
						outer.vert_01(), inner.vert_01(),
						Geom::Vec2{rect.vert_01().x + cornerSize, inner.vert_01().y},
						Geom::Vec2{rect.vert_01().x + cornerSize, outer.vert_01().y}
					);

					Fill::quad(*contextTexture,
						outer.vert_11(), inner.vert_11(),
						Geom::Vec2{inner.vert_11().x, rect.vert_11().y - cornerSize},
						Geom::Vec2{outer.vert_11().x, rect.vert_11().y - cornerSize}
					);
					Fill::quad(*contextTexture,
						outer.vert_11(), inner.vert_11(),
						Geom::Vec2{rect.vert_11().x - cornerSize, inner.vert_11().y},
						Geom::Vec2{rect.vert_11().x - cornerSize, outer.vert_11().y}
					);
				}
			}
		};
	};
}
