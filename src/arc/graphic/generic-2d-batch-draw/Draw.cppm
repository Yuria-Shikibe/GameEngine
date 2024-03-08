module;

#include <glad/glad.h>

export module Graphic.Draw;

import Math;
import Concepts;

export import Graphic.Color;

import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;
import Geom.Matrix3D;

import GL;
export import GL.Blending;
import GL.Shader;
import GL.Mesh;
import GL.Buffer.IndexBuffer;
import GL.Buffer.VertexBuffer;
import GL.Buffer.FrameBuffer;
import GL.VertexArray;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;
export  import GL.Texture.TextureRegionRect;
import RuntimeException;

import <functional>;
import <memory>;
import <stack>;

using namespace Graphic;
using namespace Geom;
using namespace GL;

namespace Graphic::Draw {
	Vec2 vec2_0{};
	Vec2 vec2_1{};
	Vec2 vec2_2{};
	Vec2 vec2_3{};
	Vec2 vec2_4{};
	Vec2 vec2_5{};
	Vec2 vec2_6{};
}

export
/**
 * \brief Main Thread Only!
 */
namespace Graphic::Draw {
	inline float vertices[VERT_LENGTH_STD] = { 0 };

	inline Color contextColor    = Colors::WHITE;
	inline Color contextMixColor = Colors::CLEAR;

	inline const TextureRegion* contextTexture = nullptr;
	inline const TextureRegion* defaultTexture = nullptr;

	inline float contextStroke = 1.0f;

	inline const Matrix3D MAT_IDT{};

	inline const Mesh* rawMesh{ nullptr };
	inline const Shader* blitter{ nullptr };

	inline std::stack<const Mesh *> formerMesh{};

	template <Concepts::Invokable<void(const Shader&)> func>
	void blit(const GL::FrameBuffer* const draw, const Shader* shader, const func& f) {
		GL::viewport(0, 0, draw->getWidth(), draw->getHeight());
		draw->bind(GL::FrameBuffer::DRAW);

		if(shader) {
			shader->bind();
			shader->applyDynamic(f);
		}

		rawMesh->bind();
		rawMesh->render(GL_TRIANGLE_FAN, 0, GL::ELEMENTS_QUAD_STRIP_LENGTH);
	}

	inline void blit(const GL::FrameBuffer* const draw, const Shader* shader = blitter) {
		GL::viewport(0, 0, draw->getWidth(), draw->getHeight());
		draw->bind(GL::FrameBuffer::DRAW);

		if(shader) {
			shader->bind();
			shader->apply();
		}

		rawMesh->bind();
		rawMesh->render(GL_TRIANGLE_FAN, 0, GL::ELEMENTS_QUAD_STRIP_LENGTH);
	}

	inline void blitCopy(const GL::FrameBuffer* const read, const GL::FrameBuffer* const draw,
	                     const GLbitfield mask = GL_COLOR_BUFFER_BIT, const GLenum filter = GL_LINEAR) {
		read->bind(GL::FrameBuffer::READ);
		draw->bind(GL::FrameBuffer::DRAW);
		glBlitFramebuffer(0, 0, read->getWidth(), read->getHeight(), 0, 0, draw->getWidth(), draw->getHeight(), mask,
		                  filter);
	}


	inline void color(const Color& color = Colors::WHITE) {
		contextColor = color;
	}

	inline void mixColor(const Color& color = Colors::CLEAR) {
		contextMixColor = color;
	}

	inline void alpha(const float a = 1.0f) {
		contextColor.setA(a);
	}

	inline void mixAlpha(const float a) {
		contextMixColor.setA(a);
	}

	inline void mixMulAlpha(const float a) {
		contextMixColor.mulA(a);
	}

	const Matrix3D& getPorj();

	void beginPorj(const Matrix3D& mat);

	void endPorj();

	void color(const Color& c1, const Color& c2, const float t) {
		contextColor.lerp(t, c1, c2);
	}

	void blend(const Blending& blending = Blendings::Normal);

	inline void texture(const TextureRegion* texture = defaultTexture) {
		contextTexture = texture;
	}

	inline void defTexture(const TextureRegion* texture) {
		defaultTexture = texture;
	}

	void flush();

	inline void meshBegin(const Mesh* const mesh) {
		mesh->bind();
		formerMesh.push(mesh);
	}

	//Use this for safety!
	inline void meshEnd(const Mesh* const mesh, const bool render = false) {
		if(render)mesh->render();
		if(mesh == formerMesh.top()) {
			formerMesh.pop();

			if(!formerMesh.empty())formerMesh.top()->bind();
		} else {
			throw ext::RuntimeException{ "Cannot end incorredt mesh!" };
		}
	}

	inline void meshEnd(const bool render = false) {
		if(render)formerMesh.top()->render();
		formerMesh.pop();

		if(!formerMesh.empty()) formerMesh.top()->bind();
	}

	void shader(Shader* shader, bool flushContext = true);

	void shader(bool flushContext = true);

	void vert(
		const Texture2D* texture,
		float x1, float y1, float texSrc1, float texDest1, const Color& c1, const Color& cm1,
		float x2, float y2, float texSrc2, float texDest2, const Color& c2, const Color& cm2,
		float x3, float y3, float texSrc3, float texDest3, const Color& c3, const Color& cm3,
		float x4, float y4, float texSrc4, float texDest4, const Color& c4, const Color& cm4
	);

	void vert_monochromeMix(
		const Texture2D* texture, const Color& cm,
		float x1, float y1, float texSrc1, float texDest1, const Color& c1,
		float x2, float y2, float texSrc2, float texDest2, const Color& c2,
		float x3, float y3, float texSrc3, float texDest3, const Color& c3,
		float x4, float y4, float texSrc4, float texDest4, const Color& c4
	);

	void vert_monochromeAll(
		const Texture2D* texture, const Color& c, const Color& cm,
		float x1, float y1, float texSrc1, float texDest1,
		float x2, float y2, float texSrc2, float texDest2,
		float x3, float y3, float texSrc3, float texDest3,
		float x4, float y4, float texSrc4, float texDest4
	);

	inline void vert(
		const Texture2D* texture,
		const float x1, const float y1, const Color& c1, const Color& cm1,
		const float x2, const float y2, const Color& c2, const Color& cm2,
		const float x3, const float y3, const Color& c3, const Color& cm3,
		const float x4, const float y4, const Color& c4, const Color& cm4
	) {
		vert(
			texture,
			x1, y1, 0.0f, 0.0f, c1, cm1,
			x2, y2, 1.0f, 0.0f, c2, cm2,
			x3, y3, 1.0f, 1.0f, c3, cm3,
			x4, y4, 0.0f, 1.0f, c4, cm4
		);
	}

	inline void vert(
		const Texture2D* texture,
		const float x1, const float y1, const Color& c1,
		const float x2, const float y2, const Color& c2,
		const float x3, const float y3, const Color& c3,
		const float x4, const float y4, const Color& c4
	) {
		vert_monochromeMix(
			texture, contextMixColor,
			x1, y1, 0.0f, 0.0f, c1,
			x2, y2, 1.0f, 0.0f, c2,
			x3, y3, 1.0f, 1.0f, c3,
			x4, y4, 0.0f, 1.0f, c4
		);
	}

	inline void vert(
		const Texture2D* texture,
		const float x1, const float y1,
		const float x2, const float y2,
		const float x3, const float y3,
		const float x4, const float y4
	) {
		vert_monochromeAll(
			texture, contextColor, contextMixColor,
			x1, y1, 0.0f, 0.0f,
			x2, y2, 1.0f, 0.0f,
			x3, y3, 1.0f, 1.0f,
			x4, y4, 0.0f, 1.0f
		);
	}

	inline void vert(
		const Texture2D* texture,
		const Vec2& v1,
		const Vec2& v2,
		const Vec2& v3,
		const Vec2& v4
	) {
		vert(
			texture,
			v1.x, v1.y,
			v2.x, v2.y,
			v3.x, v3.y,
			v4.x, v4.y
		);
	}

	inline void quad(const TextureRegion* region,
	                 const float x1, const float y1, const Color& c1,
	                 const float x2, const float y2, const Color& c2,
	                 const float x3, const float y3, const Color& c3,
	                 const float x4, const float y4, const Color& c4
	) {
		vert_monochromeMix(
			region->getData(), contextMixColor,
			x1, y1, region->u00(), region->v00(), c1,
			x2, y2, region->u10(), region->v10(), c2,
			x3, y3, region->u11(), region->v11(), c3,
			x4, y4, region->u01(), region->v01(), c4
		);
	}

	template <Concepts::Pos<float> T>
	void quad(const TextureRegion* region,
	          const T& v0, const Color& c1,
	          const T& v1, const Color& c2,
	          const T& v2, const Color& c3,
	          const T& v3, const Color& c4
	) {
		vert_monochromeMix(
			region->getData(), contextMixColor,
			v0.getX(), v0.getY(), region->u00(), region->v00(), c1,
			v1.getX(), v1.getY(), region->u10(), region->v10(), c2,
			v2.getX(), v2.getY(), region->u11(), region->v11(), c3,
			v3.getX(), v3.getY(), region->u01(), region->v01(), c4
		);
	}

	template <Concepts::Pos<float> T>
	void quad(const TextureRegion* region, const T& v0, const T& v1, const T& v2, const T& v3) {
		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			v0.getX(), v0.getY(), region->u00(), region->v00(),
			v1.getX(), v1.getY(), region->u10(), region->v10(),
			v2.getX(), v2.getY(), region->u11(), region->v11(),
			v3.getX(), v3.getY(), region->u01(), region->v01()
		);
	}

	inline void quad(const TextureRegion* region,
	                 const float x1, const float y1,
	                 const float x2, const float y2,
	                 const float x3, const float y3,
	                 const float x4, const float y4
	) {
		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			x1, y1, region->u00(), region->v00(),
			x2, y2, region->u10(), region->v10(),
			x3, y3, region->u11(), region->v11(),
			x4, y4, region->u01(), region->v01()
		);
	}

	inline void rect(const TextureRegion* region,
	                 const float x, const float y,
	                 const float w, const float h
	) {
		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			x, y, region->u00(), region->v00(),
			x + w, y, region->u10(), region->v10(),
			x + w, y + h, region->u11(), region->v11(),
			x, y + h, region->u01(), region->v01()
		);
	}

	inline void rect(const TextureRegionRect* region,
	                 const float x, const float y,
	                 const float ang
	) {
		const float sin = Math::sinDeg(ang);
		const float cos = Math::cosDeg(ang);
		const float w1  = cos * region->getWidth() * 0.5f;
		const float h1  = sin * region->getWidth() * 0.5f;

		const float w2 = -sin * region->getHeight() * 0.5f;
		const float h2 = cos * region->getHeight() * 0.5f;
		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			x - w1 - w2, y - h1 - h2, region->u00(), region->v00(),
			x + w1 - w2, y + h1 - h2, region->u10(), region->v10(),
			x + w1 + w2, y + h1 + h2, region->u11(), region->v11(),
			x - w1 + w2, y - h1 + h2, region->u01(), region->v01()
		);
	}

	void rect(
		const float x, const float y,
		const float w, const float h
	) {
		rect(defaultTexture, x, y, w, h);
	}

	inline void quad(const TextureRegion* region, const Geom::Shape::OrthoRectFloat& rect, const float x = 0,
	                 const float y                                                                       = 0) {
		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			rect.getSrcX() + x, rect.getSrcY() + y, region->u00(), region->v00(),
			rect.getSrcX() + x, rect.getEndY() + y, region->u10(), region->v10(),
			rect.getEndX() + x, rect.getEndY() + y, region->u11(), region->v11(),
			rect.getEndX() + x, rect.getSrcY() + y, region->u01(), region->v01()
		);
	}

	inline void quad(const TextureRegion* region, const Geom::Shape::OrthoRectFloat& rect) {
		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			rect.getSrcX(), rect.getSrcY(), region->u00(), region->v00(),
			rect.getSrcX(), rect.getEndY(), region->u10(), region->v10(),
			rect.getEndX(), rect.getEndY(), region->u11(), region->v11(),
			rect.getEndX(), rect.getSrcY(), region->u01(), region->v01()
		);
	}


	inline void quad(
		const float x1, const float y1,
		const float x2, const float y2,
		const float x3, const float y3,
		const float x4, const float y4
	) {
		vert_monochromeAll(
			defaultTexture->getData(), contextColor, contextMixColor,
			x1, y1, defaultTexture->u00(), defaultTexture->v00(),
			x2, y2, defaultTexture->u10(), defaultTexture->v10(),
			x3, y3, defaultTexture->u11(), defaultTexture->v11(),
			x4, y4, defaultTexture->u01(), defaultTexture->v01()
		);
	}

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

	inline void reset() {
		contextStroke = 1.0f;
		color();
		mixColor();
		texture();
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

	void rectLine(const float srcx, const float srcy, const float width, const float height, const bool cap = true) {
		line(defaultTexture, srcx, srcy, srcx, srcy + height - contextStroke, contextColor, contextColor, cap);
		line(defaultTexture, srcx, srcy + height, srcx + width - contextStroke, srcy + height, contextColor, contextColor, cap);
		line(defaultTexture, srcx + width, srcy + height, srcx + width, srcy + contextStroke, contextColor, contextColor, cap);
		line(defaultTexture, srcx + width, srcy, srcx + contextStroke, srcy, contextColor, contextColor, cap);
	}

	void rectLine(const Geom::Shape::OrthoRectFloat& rect, const bool cap = true, const Vec2& offset = Geom::ZERO) {
		rectLine(rect.getSrcX() + offset.getX(), rect.getSrcY() + offset.getY(), rect.getWidth(), rect.getHeight(),
		         cap);
	}

	void lineSquare(const float x, const float y, const float radius, float ang) {
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

	void fillSquare(const float x, const float y, const float radius, float ang) {
		ang += 45.000f;

		vec2_0.setPolar(ang, radius / Math::SQRT2);

		vec2_1.set(x, y).add(vec2_0);
		vec2_2.set(x, y).add(vec2_0.rotateRT());

		vec2_3.set(x, y).add(vec2_0.rotateRT());
		vec2_4.set(x, y).add(vec2_0.rotateRT());

		Draw::quad(defaultTexture, vec2_1, vec2_2, vec2_3, vec2_4);
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

	template <size_t size>
	void poly(const float x, const float y, const int sides, const float radius, const float angle, const float ratio,
	          const Color* const(&colorGroup)[size]) {
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
		Color lerpColor1 = *colorGroup[0x000000];
		Color lerpColor2 = *colorGroup[size - 1];

		for(; progress < fSides * ratio - 1.0f; progress += 1.0f) {
			// NOLINT(cert-flp30-c)
			currentAng = angle + (progress + 1.0f) * space;

			sin2 = Math::sinDeg(currentAng);
			cos2 = Math::cosDeg(currentAng);

			currentRatio = progress / fSides;

			lerpColor2.lerp(colorGroup, currentRatio);

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

		lerpColor2.lerp(colorGroup, progress / fSides).lerp(lerpColor1, 1.0f - remainRatio);

		quad(defaultTexture,
		     cos1 * r1 + x, sin1 * r1 + y, lerpColor1,
		     cos1 * r2 + x, sin1 * r2 + y, lerpColor1,
		     cos2 * r2 + x, sin2 * r2 + y, lerpColor2,
		     cos2 * r1 + x, sin2 * r1 + y, lerpColor2
		);
	}
}

