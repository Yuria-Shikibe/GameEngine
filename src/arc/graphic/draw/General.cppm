//
// Created by Matrix on 2024/3/19.
//

export module Graphic.Draw;

export import GL.Constants;
export import GL.Blending;
export import GL;
export import GL.Texture;
export import GL.Texture.TextureRegion;
export import GL.Texture.TextureRegionRect;

import GL.Mesh;
import GL.Shader;
import GL.Buffer.FrameBuffer;


export import Geom.Transform;
export import Geom.Shape.RectBox;
export import Geom.Shape.Rect_Orthogonal;
export import Geom.Matrix3D;

export import Graphic.Color;

import Math;
import std;
import RuntimeException;

import Core.BatchGroup;

namespace Graphic::Draw{
	using namespace Geom;
	//TEMP!
	thread_local Vec2 vec2_0{};
	thread_local Vec2 vec2_1{};
	thread_local Vec2 vec2_2{};
	thread_local Vec2 vec2_3{};
	thread_local Vec2 vec2_4{};
	thread_local Vec2 vec2_5{};
	thread_local Vec2 vec2_6{};

	namespace Line{
		float contextStroke = 1.0f;
		std::vector<Geom::Vec2> pointBuffer{};

		bool buildingLine = false;
		bool closedLine = false;

		inline Color beginColor{};
		inline Color endColor{};
	}
}

export namespace Graphic{
	using namespace GL;
	using namespace Geom;

	namespace World{
		void vert(float* vertices,
		          float x1, float y1, float z1, float u1, float v1, Color c1, Color cm1,
		          float x2, float y2, float z2, float u2, float v2, Color c2, Color cm2,
		          float x3, float y3, float z3, float u3, float v3, Color c3, Color cm3,
		          float x4, float y4, float z4, float u4, float v4, Color c4, Color cm4
		);

		void vert_monochromeMix(float* vertices, Color cm,
		                        float x1, float y1, float z1, float u1, float v1, Color c1,
		                        float x2, float y2, float z2, float u2, float v2, Color c2,
		                        float x3, float y3, float z3, float u3, float v3, Color c3,
		                        float x4, float y4, float z4, float u4, float v4, Color c4
		);

		void vert_monochromeAll(float* vertices, Color c, Color cm,
		                        float x1, float y1, float z1, float u1, float v1,
		                        float x2, float y2, float z2, float u2, float v2,
		                        float x3, float y3, float z3, float u3, float v3,
		                        float x4, float y4, float z4, float u4, float v4
		);
	}

	namespace Overlay{
		void vert(float* vertices,
		          float x1, float y1, float u1, float v1, Color c1, Color cm1,
		          float x2, float y2, float u2, float v2, Color c2, Color cm2,
		          float x3, float y3, float u3, float v3, Color c3, Color cm3,
		          float x4, float y4, float u4, float v4, Color c4, Color cm4
		);

		void vert_monochromeMix(float* vertices, Color cm,
		                        float x1, float y1, float u1, float v1, Color c1,
		                        float x2, float y2, float u2, float v2, Color c2,
		                        float x3, float y3, float u3, float v3, Color c3,
		                        float x4, float y4, float u4, float v4, Color c4
		);

		void vert_monochromeAll(float* vertices, Color c, Color cm,
		                        float x1, float y1, float u1, float v1,
		                        float x2, float y2, float u2, float v2,
		                        float x3, float y3, float u3, float v3,
		                        float x4, float y4, float u4, float v4
		);
	}

	using BatchPtr = std::unique_ptr<Core::Batch>;

	constexpr auto OverlayBatch = &Core::BatchGroup::batchOverlay;
	constexpr auto WorldBatch = &Core::BatchGroup::batchWorld;
	constexpr auto DefBatch = OverlayBatch;

	extern Core::BatchGroup& batchGroup;

	template <BatchPtr Core::BatchGroup::* batchPtr>
	BatchPtr& getBatch(){
		return batchGroup.*batchPtr;
	}

	template <BatchPtr Core::BatchGroup::* batchPtr>
	struct VertexPasser;

	template <>
	struct VertexPasser<&Core::BatchGroup::batchWorld> {
		static constexpr auto batchPtr = &Core::BatchGroup::batchWorld;

		static constexpr int size = VERT_LENGTH_WORLD;

		inline static float vertices[size]{};

		static void vert(const Texture* texture, const auto... args){
			World::vert(vertices, args...);
			getBatch<batchPtr>()->post(texture, vertices, 0, size);
		}

		static void vert_monochromeMix(const Texture* texture, const auto... args){
			World::vert_monochromeMix(vertices, args...);
			getBatch<batchPtr>()->post(texture, vertices, 0, size);
		}

		static void vert_monochromeAll(const Texture* texture, const auto... args){
			World::vert_monochromeAll(vertices, args...);
			getBatch<batchPtr>()->post(texture, vertices, 0, size);
		}
	};

	template <>
	struct VertexPasser<&Core::BatchGroup::batchOverlay> {
		static constexpr auto batchPtr = &Core::BatchGroup::batchOverlay;

		static constexpr int size = VERT_LENGTH_OVERLAY;

		inline static float vertices[size]{};

		static void vert(const Texture* texture, const auto... args){
			Overlay::vert(vertices, args...);
			getBatch<batchPtr>()->post(texture, vertices, 0, size);
		}

		static void vert_monochromeMix(const Texture* texture, const auto... args){
			Overlay::vert_monochromeMix(vertices, args...);
			getBatch<batchPtr>()->post(texture, vertices, 0, size);
		}

		static void vert_monochromeAll(const Texture* texture, const auto... args){
			Overlay::vert_monochromeAll(vertices, args...);
			getBatch<batchPtr>()->post(texture, vertices, 0, size);
		}
	};

	template <BatchPtr Core::BatchGroup::* batchPtr>
	constexpr auto vertGroupSize = VertexPasser<batchPtr>::size;

	inline constexpr float circleVertPrecision{ 8 };

	constexpr int getCircleVerts(const float radius){
		return Math::max(Math::ceil(radius * Math::PI / circleVertPrecision), 12);
	}


	inline std::stack<const Mesh*> formerMesh{};

	// template <typename HandleType>
	// void vert_raw(const Texture* texture, const auto... args){
	// 	HandleType::vert(args...);
	//
	// 	getBatch<HandleType::batchPtr>()->post(texture, HandleType::vertices, 0, HandleType::size);
	// }
	//
	// template <typename HandleType>
	// void vert_raw_monochromeMix(const Texture* texture, const auto... args){
	// 	HandleType::vert_monochromeMix(args...);
	//
	// 	getBatch<HandleType::batchPtr>()->post(texture, HandleType::vertices, 0, HandleType::size);
	// }
	//
	// template <typename HandleType>
	// void vert_raw_monochromeAll(const Texture* texture, const auto... args){
	// 	HandleType::vert_monochromeAll(args...);
	//
	// 	getBatch<HandleType::batchPtr>()->post(texture, HandleType::vertices, 0, HandleType::size);
	// }


	namespace Frame{
		inline const Mesh* rawMesh{ nullptr };
		inline const Shader* blitter{ nullptr };

		template <Concepts::InvokeNullable<void(const Shader&)> Func = std::nullptr_t>
		void blit(const GL::FrameBuffer* const draw, const unsigned port = 0, const Shader* shader = blitter,
		          Func&& f = nullptr){
			GL::viewport(0, 0, draw->getWidth(), draw->getHeight());
			draw->bind(GL::FrameBuffer::DRAW);
			draw->enableDrawAt(port);

			if(shader){
				shader->bind();
				if constexpr(!std::is_same_v<std::nullptr_t, Func>){
					shader->applyDynamic(f);
				} else{
					shader->apply();
				}
			}

			rawMesh->bind();
			rawMesh->render(GL_TRIANGLE_FAN, 0, GL::ELEMENTS_QUAD_STRIP_LENGTH);
		}

		void blitCopyAll(const GL::FrameBuffer* const read, const GL::FrameBuffer* const draw,
		                 const GLbitfield mask = GL_COLOR_BUFFER_BIT, const GLenum filter = GL_LINEAR){
			const unsigned int max = std::min(read->getTextures().size(), draw->getTextures().size());

			read->bind(GL::FrameBuffer::READ);
			draw->bind(GL::FrameBuffer::DRAW);

			for(int i = 0; i < max; ++i){
				read->enableRead(i);
				draw->enableDrawAt(i);

				GL::blit(
					0, 0, read->getWidth(), read->getHeight(),
					0, 0, draw->getWidth(), draw->getHeight(),
					mask, filter);
			}
		}

		void blitCopy(const GL::FrameBuffer* const read, unsigned readAttachmentID, const GL::FrameBuffer* const draw,
		              unsigned drawAttachmentID,
		              const GLbitfield mask = GL_COLOR_BUFFER_BIT, const GLenum filter = GL_LINEAR){
			read->bind(GL::FrameBuffer::READ);
			draw->bind(GL::FrameBuffer::DRAW);

			read->enableRead(readAttachmentID);
			draw->enableDrawAt(drawAttachmentID);

			GL::blit(
				0, 0, read->getWidth(), read->getHeight(),
				0, 0, draw->getWidth(), draw->getHeight(),
				mask, filter);
		}
	}

	namespace Batch{
		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		const Matrix3D* getPorj(){
			return getBatch<batchPtr>()->getProjection();
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void beginPorj(const Matrix3D& mat){
			getBatch<batchPtr>()->beginTempProjection(mat);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void endPorj(){
			getBatch<batchPtr>()->endTempProjection();
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void blend(const GL::Blending& blending = Blendings::Normal){
			getBatch<batchPtr>()->switchBlending(blending);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void flush(){
			getBatch<batchPtr>()->flush();
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void shader(Shader* shader, const bool flushContext){
			if(flushContext) flush<batchPtr>();

			getBatch<batchPtr>()->setCustomShader(shader);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void shader(const bool flushContext = true){
			getBatch<batchPtr>()->clearCustomShader(flushContext);
		}
	}

	namespace Mesh{
		inline void meshBegin(const GL::Mesh* const mesh){
			mesh->bind();
			formerMesh.push(mesh);
		}

		//Use this for safety!
		inline void meshEnd(const GL::Mesh* const mesh, const bool render = false){
			if(render) mesh->render();
			if(mesh == formerMesh.top()){
				formerMesh.pop();

				if(!formerMesh.empty()) formerMesh.top()->bind();
			} else{
				throw ext::RuntimeException{ "Cannot end incorredt mesh!" };
			}
		}

		inline void meshEnd(const bool render = false){
			if(render) formerMesh.top()->render();
			formerMesh.pop();

			if(!formerMesh.empty()) formerMesh.top()->bind();
		}
	}

	namespace Draw{
		constexpr float DepthNear = 1;
		constexpr float DepthFar = 300;

		//TODO necessity to move these context value to batch
		//TODO concurrency support to remove these context value
		inline Color contextColor = Colors::WHITE;
		inline Color contextMixColor = Colors::CLEAR;

		inline const TextureRegion* contextTexture = nullptr;
		inline const TextureRegion* defaultTexture = nullptr;

		inline const TextureRegion* defaultLightTexture = nullptr;
		inline const TextureRegion* defaultSolidTexture = nullptr;

		inline float contextNorZ = 1.0f;

		constexpr float getNormalizedDepth(const float z){
			return (1 / z - 1 / DepthNear) / (1 / DepthFar - 1 / DepthNear);
		}

		void setZ(const float z){ contextNorZ = getNormalizedDepth(z); }

		void setNorZ(const float z){ contextNorZ = z; }

		[[nodiscard]] float getNorZ(){ return contextNorZ; }

		template <bool applyAlpha = true>
		void color(const Color color = Colors::WHITE){
			if constexpr(applyAlpha){
				contextColor = color;
			}else{
				contextColor.set(color.r, color.g, color.b);
			}
		}

		template <bool applyAlpha = true>
		void mixColor(const Color color = Colors::CLEAR){
			if constexpr(applyAlpha){
				contextMixColor = color;
			}else{
				contextMixColor.set(color.r, color.g, color.b);
			}
		}

		void alpha(const float a = 1.0f){ contextColor.setA(a); }

		void mixAlpha(const float a){ contextMixColor.setA(a); }

		void mixMulAlpha(const float a){ contextMixColor.mulA(a); }

		void color(const Color c1, const Color c2, const float t){ contextColor.lerp(t, c1, c2); }

		void setDefTexture(const TextureRegion* texture){ defaultTexture = texture; }

		void setTexture(const TextureRegion* texture = defaultTexture){ contextTexture = texture; }

		inline void reset(){
			color();
			mixColor();
			setTexture();
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void vert(
			const Texture* texture,
			const float x1, const float y1, const float u1, const float v1, const Color c1, const Color cm1,
			const float x2, const float y2, const float u2, const float v2, const Color c2, const Color cm2,
			const float x3, const float y3, const float u3, const float v3, const Color c3, const Color cm3,
			const float x4, const float y4, const float u4, const float v4, const Color c4, const Color cm4
		){
			if constexpr(batchPtr == WorldBatch){
				VertexPasser<WorldBatch>::vert(texture,
				                   x1, y1, contextNorZ, u1, v1, c1, cm1, x2, y2, contextNorZ, u2, v2, c2, cm2,
				                   x3, y3, contextNorZ, u3, v3, c3, cm3, x4, y4, contextNorZ, u4, v4, c4, cm4
				);
			} else{
				VertexPasser<OverlayBatch>::vert(texture,
				                   x1, y1, u1, v1, c1, cm1, x2, y2, u2, v2, c2, cm2,
				                   x3, y3, u3, v3, c3, cm3, x4, y4, u4, v4, c4, cm4
				);
			}
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void vert_monochromeMix(
			const Texture* texture, const Color cm,
			const float x1, const float y1, const float u1, const float v1, const Color c1,
			const float x2, const float y2, const float u2, const float v2, const Color c2,
			const float x3, const float y3, const float u3, const float v3, const Color c3,
			const float x4, const float y4, const float u4, const float v4, const Color c4
		){
			if constexpr(batchPtr == WorldBatch){
				VertexPasser<WorldBatch>::vert_monochromeMix(texture, cm,
				                                 x1, y1, contextNorZ, u1, v1, c1, x2, y2, contextNorZ, u2, v2, c2,
				                                 x3, y3, contextNorZ, u3, v3, c3, x4, y4, contextNorZ, u4, v4, c4
				);
			} else{
				VertexPasser<OverlayBatch>::vert_monochromeMix(texture, cm,
				                                 x1, y1, u1, v1, c1, x2, y2, u2, v2, c2,
				                                 x3, y3, u3, v3, c3, x4, y4, u4, v4, c4
				);
			}
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void vert_monochromeAll(
			const Texture* texture, const Color cm, const Color c,
			const float x1, const float y1, const float u1, const float v1,
			const float x2, const float y2, const float u2, const float v2,
			const float x3, const float y3, const float u3, const float v3,
			const float x4, const float y4, const float u4, const float v4
		){
			if constexpr(batchPtr == WorldBatch){
				VertexPasser<WorldBatch>::vert_monochromeAll(texture, cm, c,
				                                 x1, y1, contextNorZ, u1, v1, x2, y2, contextNorZ, u2, v2,
				                                 x3, y3, contextNorZ, u3, v3, x4, y4, contextNorZ, u4, v4
				);
			} else{
				VertexPasser<OverlayBatch>::vert_monochromeAll(texture, cm, c,
				                                 x1, y1, u1, v1, x2, y2, u2, v2,
				                                 x3, y3, u3, v3, x4, y4, u4, v4
				);
			}
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(const TextureRegion* region,
		          const float x1, const float y1, const Color c1,
		          const float x2, const float y2, const Color c2,
		          const float x3, const float y3, const Color c3,
		          const float x4, const float y4, const Color c4
		){
			vert_monochromeMix<batchPtr>(
				region->getData(), contextMixColor,
				x1, y1, region->u00(), region->v00(), c1,
				x2, y2, region->u10(), region->v10(), c2,
				x3, y3, region->u11(), region->v11(), c3,
				x4, y4, region->u01(), region->v01(), c4
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(const TextureRegion* region,
		          const Vec2 v0, const Color c1,
		          const Vec2 v1, const Color c2,
		          const Vec2 v2, const Color c3,
		          const Vec2 v3, const Color c4
		){
			::Graphic::Draw::vert_monochromeMix<batchPtr>(
				region->getData(), contextMixColor,
				v0.x, v0.y, region->u00(), region->v00(), c1,
				v1.x, v1.y, region->u10(), region->v10(), c2,
				v2.x, v2.y, region->u11(), region->v11(), c3,
				v3.x, v3.y, region->u01(), region->v01(), c4
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(const TextureRegion* region,
			const Concepts::Pos<float> auto& v0, const Concepts::Pos<float> auto& v1,
			const Concepts::Pos<float> auto& v2, const Concepts::Pos<float> auto& v3
		){
			::Graphic::Draw::vert_monochromeAll<batchPtr>(
				region->getData(), contextColor, contextMixColor,
				v0.getX(), v0.getY(), region->u00(), region->v00(),
				v1.getX(), v1.getY(), region->u10(), region->v10(),
				v2.getX(), v2.getY(), region->u11(), region->v11(),
				v3.getX(), v3.getY(), region->u01(), region->v01()
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(
			const Concepts::Pos<float> auto& v0, const Concepts::Pos<float> auto& v1,
			const Concepts::Pos<float> auto& v2, const Concepts::Pos<float> auto& v3
		){
			::Graphic::Draw::vert_monochromeAll<batchPtr>(
				contextTexture->getData(), contextColor, contextMixColor,
				v0.getX(), v0.getY(), contextTexture->u00(), contextTexture->v00(),
				v1.getX(), v1.getY(), contextTexture->u10(), contextTexture->v10(),
				v2.getX(), v2.getY(), contextTexture->u11(), contextTexture->v11(),
				v3.getX(), v3.getY(), contextTexture->u01(), contextTexture->v01()
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(const TextureRegion* region,
		          const float x1, const float y1,
		          const float x2, const float y2,
		          const float x3, const float y3,
		          const float x4, const float y4
		){
			::Graphic::Draw::vert_monochromeAll<batchPtr>(
				region->getData(), contextColor, contextMixColor,
				x1, y1, region->u00(), region->v00(),
				x2, y2, region->u10(), region->v10(),
				x3, y3, region->u11(), region->v11(),
				x4, y4, region->u01(), region->v01()
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void rectOrtho(const TextureRegion* region,
		          const float x, const float y,
		          const float w, const float h
		){
			vert_monochromeAll<batchPtr>(
				region->getData(), contextColor, contextMixColor,
				x, y, region->u00(), region->v00(),
				x + w, y, region->u10(), region->v10(),
				x + w, y + h, region->u11(), region->v11(),
				x, y + h, region->u01(), region->v01()
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void rectOrtho(const TextureRegion* region, const Geom::OrthoRectFloat rect){
			rectOrtho<batchPtr>(region, rect.getSrcX(), rect.getSrcY(), rect.getWidth(), rect.getHeight());
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void rect(const TextureRegionRect* region,
		          const float x, const float y,
		          const float ang
		){
			const float sin = Math::sinDeg(ang);
			const float cos = Math::cosDeg(ang);
			const float w1 = cos * region->getWidth() * 0.5f;
			const float h1 = sin * region->getWidth() * 0.5f;

			const float w2 = -sin * region->getHeight() * 0.5f;
			const float h2 = cos * region->getHeight() * 0.5f;
			vert_monochromeAll<batchPtr>(
				region->getData(), contextColor, contextMixColor,
				x - w1 - w2, y - h1 - h2, region->u00(), region->v00(),
				x + w1 - w2, y + h1 - h2, region->u10(), region->v10(),
				x + w1 + w2, y + h1 + h2, region->u11(), region->v11(),
				x - w1 + w2, y - h1 + h2, region->u01(), region->v01()
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void rect(const TextureRegionRect* region, auto trans) requires Concepts::Derived<decltype(trans), Geom::Transform> {
			::Graphic::Draw::rect<batchPtr>(region, trans.vec.x, trans.vec.y, trans.rot);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void rectOrtho(const float x, const float y, const float w, const float h){
			rectOrtho<batchPtr>(contextTexture, x, y, w, h);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(const TextureRegion* region, const Geom::OrthoRectFloat rect, const float x = 0,
		          const float y = 0){
			vert_monochromeAll<batchPtr>(
				region->getData(), contextColor, contextMixColor,
				rect.getSrcX() + x, rect.getSrcY() + y, region->u00(), region->v00(),
				rect.getSrcX() + x, rect.getEndY() + y, region->u10(), region->v10(),
				rect.getEndX() + x, rect.getEndY() + y, region->u11(), region->v11(),
				rect.getEndX() + x, rect.getSrcY() + y, region->u01(), region->v01()
			);
		}

		template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
		void quad(
			const float x1, const float y1,
			const float x2, const float y2,
			const float x3, const float y3,
			const float x4, const float y4
		){
			vert_monochromeAll<batchPtr>(
				contextTexture->getData(), contextColor, contextMixColor,
				x1, y1, contextTexture->u00(), contextTexture->v00(),
				x2, y2, contextTexture->u10(), contextTexture->v10(),
				x3, y3, contextTexture->u11(), contextTexture->v11(),
				x4, y4, contextTexture->u01(), contextTexture->v01()
			);
		}

		namespace Fill{
			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void square(const float x, const float y, const float radius, const float ang){
				vec2_0.setPolar(ang + 45.0f, radius / Math::SQRT2);

				vec2_1.set(x, y).add(vec2_0);
				vec2_2.set(x, y).add(vec2_0.rotateRT());

				vec2_3.set(x, y).add(vec2_0.rotateRT());
				vec2_4.set(x, y).add(vec2_0.rotateRT());

				quad<batchPtr>(contextTexture, vec2_1, vec2_2, vec2_3, vec2_4);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void poly(const float x, const float y, const int sides, const float radius, const float angle,
			          const Color inner = contextColor, const Color exter = contextColor){
				const float space = 360.0f / static_cast<float>(sides);

				for(int i = 0; i < sides; i++){
					const float a = space * static_cast<float>(i) + angle;
					const float cos1 = Math::cosDeg(a);
					const float sin1 = Math::sinDeg(a);
					const float cos2 = Math::cosDeg(a + space);
					const float sin2 = Math::sinDeg(a + space);
					quad<batchPtr>(
						contextTexture,
						x, y, inner,
						x, y, inner,
						x + radius * cos2, y + radius * sin2, exter,
						x + radius * cos1, y + radius * sin1, exter
					);
				}
			}
		}

		namespace Line{
			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void line(const TextureRegion* region, const float x, const float y, const float x2, const float y2,
			          const Color& c1 = contextColor, const Color& c2 = contextColor, const bool cap = true){
				const float h_stroke = contextStroke / 2.0f;
				const float len = Math::len(x2 - x, y2 - y);
				const float diff_x = (x2 - x) / len * h_stroke;
				const float diff_y = (y2 - y) / len * h_stroke;

				if(cap){
					quad<batchPtr>(
						region,
						x - diff_x - diff_y, y - diff_y + diff_x, c1,
						x - diff_x + diff_y, y - diff_y - diff_x, c1,
						x2 + diff_x + diff_y, y2 + diff_y - diff_x, c2,
						x2 + diff_x - diff_y, y2 + diff_y + diff_x, c2
					);
				} else{
					quad<batchPtr>(
						region,
						x - diff_y, y + diff_x, c1,
						x + diff_y, y - diff_x, c1,
						x2 + diff_y, y2 - diff_x, c2,
						x2 - diff_y, y2 + diff_x, c2
					);
				}
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void line(const float x, const float y, const float x2, const float y2, const bool cap = true){
				line<batchPtr>(contextTexture, x, y, x2, y2, contextColor, contextColor, cap);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void line(const Vec2 v1, const Vec2 v2, const Color& c1 = contextColor,
			                    const Color& c2 = contextColor,
			                    const bool cap = true){
				line<batchPtr>(contextTexture, v1.x, v1.y, v2.x, v2.y, c1, c2, cap);
			}


			void setLineStroke(const float s){
				contextStroke = s;
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void lineAngleCenter(const float x, const float y, const float angle, const float length,
			                     const bool cap = true){
				vec2_0.setPolar(angle, length * 0.5f);

				line<batchPtr>(contextTexture, x - vec2_0.x, y - vec2_0.y, x + vec2_0.x, y + vec2_0.y, contextColor,
				               contextColor,
				               cap);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void lineAngle(const float x, const float y, const float angle, const float length,
			               const bool cap = true){
				vec2_0.setPolar(angle, length);

				line<batchPtr>(contextTexture, x, y, x + vec2_0.x, y + vec2_0.y, contextColor, contextColor, cap);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void lineAngle(const float x, const float y, const float angle, const float length,
			               const float offset){
				vec2_0.setPolar(angle, 1.0f);

				line<batchPtr>(contextTexture, x + vec2_0.x * offset, y + vec2_0.y * offset,
				               x + vec2_0.x * (length + offset),
				               y + vec2_0.y * (length + offset));
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void quad(const Geom::QuadBox& box, const bool cap = true){
				for(int i = 0; i < 4; ++i) {
					line<batchPtr>(box[i], box[(i + 1) % 4], contextColor, contextColor, cap);
				}
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void rect(const float srcx, const float srcy, const float width, const float height, const bool cap = true){
				line<batchPtr>(contextTexture, srcx, srcy, srcx, srcy + height - contextStroke, contextColor,
							   contextColor, cap);
				line<batchPtr>(contextTexture, srcx, srcy + height, srcx + width - contextStroke, srcy + height,
							   contextColor,
							   contextColor, cap);
				line<batchPtr>(contextTexture, srcx + width, srcy + height, srcx + width, srcy + contextStroke,
							   contextColor,
							   contextColor, cap);
				line<batchPtr>(contextTexture, srcx + width, srcy, srcx + contextStroke, srcy, contextColor,
							   contextColor, cap);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void rect(const Geom::OrthoRectFloat& rect, const bool cap = true, const Vec2& offset = Geom::ZERO){
				Line::rect<batchPtr>(rect.getSrcX() + offset.getX(), rect.getSrcY() + offset.getY(), rect.getWidth(),
				                     rect.getHeight(),
				                     cap);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void square(const float x, const float y, const float radius, float ang){
				ang += 45.000f;
				const float dst = contextStroke / Math::SQRT2;

				vec2_0.setPolar(ang, 1);

				vec2_1.set(vec2_0);
				vec2_2.set(vec2_0);

				vec2_1.scl(radius - dst);
				vec2_2.scl(radius + dst);

				for(int i = 0; i < 4; ++i){
					vec2_0.rotateRT();

					vec2_3.set(vec2_0).scl(radius - dst);
					vec2_4.set(vec2_0).scl(radius + dst);

					::Graphic::Draw::quad<batchPtr>(vec2_1.x + x, vec2_1.y + y, vec2_2.x + x, vec2_2.y + y, vec2_4.x + x, vec2_4.y + y,
					               vec2_3.x + x,
					               vec2_3.y + y);

					vec2_1.set(vec2_3);
					vec2_2.set(vec2_4);
				}
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void poly(const float x, const float y, const int sides, const float radius, const float angle){
				const float space = 360.0f / static_cast<float>(sides);
				const float h_step = contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
				const float r1 = radius - h_step;
				const float r2 = radius + h_step;

				for(int i = 0; i < sides; i++){
					const float a = space * static_cast<float>(i) + angle;
					const float cos1 = Math::cosDeg(a);
					const float sin1 = Math::sinDeg(a);
					const float cos2 = Math::cosDeg(a + space);
					const float sin2 = Math::sinDeg(a + space);
					::Graphic::Draw::quad<batchPtr>(
						x + r1 * cos1, y + r1 * sin1,
						x + r1 * cos2, y + r1 * sin2,
						x + r2 * cos2, y + r2 * sin2,
						x + r2 * cos1, y + r2 * sin1
					);
				}
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void poly(const float x, const float y, const int sides, const float radius, const float angle,
			          const float ratio,
			          const auto&... args){
				constexpr auto size = sizeof...(args);
				const auto colors = std::make_tuple(args...);

				const auto fSides = static_cast<float>(sides);

				const float space = 360.0f / fSides;
				const float h_step = contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
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

					::Graphic::Draw::quad<batchPtr>(contextTexture,
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

				::Graphic::Draw::quad<batchPtr>(contextTexture,
				               cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
				               cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
				               cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
				               cos2 * r1 + x, sin2 * r1 + y, lerpColor2
				);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void poly(const float x, const float y, const int sides, const float radius, const float angle,
			          const float ratio,
			          const std::span<const Color>& colorGroup){
				const auto size = colorGroup.size();

				const auto fSides = static_cast<float>(sides);

				const float space = 360.0f / fSides;
				const float h_step = contextStroke / 2.0f / Math::cosDeg(space / 2.0f);
				const float r1 = radius - h_step;
				const float r2 = radius + h_step;

				float currentRatio = 0;

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

					::Graphic::Draw::quad<batchPtr>(contextTexture,
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

				::Graphic::Draw::quad<batchPtr>(contextTexture,
				               cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
				               cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
				               cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
				               cos2 * r1 + x, sin2 * r1 + y, lerpColor2
				);
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void circle(const float x, const float y, const float radius){
				poly<batchPtr>(x, y, getCircleVerts(radius), radius, 0);
			}


			void setLerpColor(const Color begin = contextColor, const Color end = contextColor){
				beginColor = begin;
				endColor = end;
			}

			//TODO support a color seq maybe in the future
			void beginLineVert(const bool closed = false){
				pointBuffer.clear();
				buildingLine = true;
				closedLine = closed;
			}

			void push(const Geom::Vec2 vec2){
				pointBuffer.push_back(vec2);
			}

			void push(const std::span<Geom::Vec2> verts){
				for(const auto vec2 : verts){
					pointBuffer.push_back(vec2);
				}
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch, Concepts::InvokeNullable<void(
				Geom::Vec2, Graphic::Color)> Func = std::nullptr_t>
			void endLineVert(Func&& func = nullptr){
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

					line<batchPtr>(pointBuffer[currentIndex], pointBuffer[currentIndex + 1], lineBeginLerp,
					               lineEndLerp);
					if constexpr(!std::is_same_v<Func, std::nullptr_t>){
						func(pointBuffer[currentIndex], lineBeginLerp);
					}
				}

				if constexpr(!std::is_same_v<Func, std::nullptr_t>){
					func(pointBuffer[currentIndex], lineEndLerp);
				}
			}

			template <BatchPtr Core::BatchGroup::* batchPtr, typename... Args>
				requires (std::is_same_v<Geom::Vec2, Args> && ...)
			void outline(const bool close, Args... args){
				beginLineVert(close);
				(::Graphic::Draw::Line::push(args), ...);
				endLineVert<batchPtr>();
			}

			template <BatchPtr Core::BatchGroup::* batchPtr = DefBatch>
			void outline(const std::span<Geom::Vec2> verts, const bool close = false){
				beginLineVert(close);
				for(const auto p : verts){
					push(p);
				}
				endLineVert<batchPtr>();
			}
		}
	}
}
