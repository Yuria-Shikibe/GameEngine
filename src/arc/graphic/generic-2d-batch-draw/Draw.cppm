export module Graphic.Draw;

import Math;
import Concepts;

export import Graphic.Color;

import Geom.Vector2D;
import Geom.Shape.Rect_Orthogonal;
import Geom.Matrix3D;

import GL;
export import GL.Blending;
import GL.Constants;
import GL.Shader;
import GL.Mesh;
import GL.Buffer.DataBuffer;
import GL.Buffer.FrameBuffer;
import GL.VertexArray;
import GL.Texture;
import GL.Texture.TextureRegion;
export import GL.Texture.TextureRegionRect;
import RuntimeException;

import std;

using namespace Graphic;
using namespace Geom;
using namespace GL;

namespace Graphic::Draw{
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
namespace Graphic::Draw{
	inline float vertices[VERT_LENGTH_STD] = { 0 };

	inline Color contextColor    = Colors::WHITE;
	inline Color contextMixColor = Colors::CLEAR;

	inline const TextureRegion* contextTexture = nullptr;
	inline const TextureRegion* defaultTexture = nullptr;

	inline float contextStroke = 1.0f;

	inline float circleVertPrecision{8};

	inline const Matrix3D MAT_IDT{};

	inline const Mesh* rawMesh{ nullptr };
	inline const Shader* blitter{ nullptr };

	inline std::stack<const Mesh *> formerMesh{};

	constexpr int getCircleVerts(const float radius){
		return Math::max(Math::ceil(radius * Math::PI / circleVertPrecision), 12);
	}

	template <Concepts::InvokeNullable<void(const Shader&)> Func = std::nullptr_t>
	void blit(const GL::FrameBuffer* const draw, const unsigned port = 0, const Shader* shader = blitter, Func&& f = nullptr) {
		GL::viewport(0, 0, draw->getWidth(), draw->getHeight());
		draw->bind(GL::FrameBuffer::DRAW);
		draw->enableDrawAt(port);

		if(shader) {
			shader->bind();
			if constexpr (!std::is_same_v<std::nullptr_t, Func>){
				shader->applyDynamic(f);
			}else{
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

	void blitCopy(const GL::FrameBuffer* const read, unsigned readAttachmentID, const GL::FrameBuffer* const draw, unsigned drawAttachmentID,
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


	inline void color(const Color color = Colors::WHITE) {
		contextColor = color;
	}

	inline void mixColor(const Color color = Colors::CLEAR) {
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

	void color(const Color c1, const Color c2, const float t) {
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

		if(!formerMesh.empty())formerMesh.top()->bind();
	}

	void shader(Shader* shader, bool flushContext = true);

	void shader(bool flushContext = true);

	void vert(
		const Texture* texture,
		float x1, float y1, float texSrc1, float texDest1, const Color c1, const Color cm1,
		float x2, float y2, float texSrc2, float texDest2, const Color c2, const Color cm2,
		float x3, float y3, float texSrc3, float texDest3, const Color c3, const Color cm3,
		float x4, float y4, float texSrc4, float texDest4, const Color c4, const Color cm4
	);

	void vert_monochromeMix(
		const Texture* texture, const Color cm,
		float x1, float y1, float texSrc1, float texDest1, const Color c1,
		float x2, float y2, float texSrc2, float texDest2, const Color c2,
		float x3, float y3, float texSrc3, float texDest3, const Color c3,
		float x4, float y4, float texSrc4, float texDest4, const Color c4
	);

	void vert_monochromeAll(
		const Texture* texture, const Color c, const Color cm,
		float x1, float y1, float texSrc1, float texDest1,
		float x2, float y2, float texSrc2, float texDest2,
		float x3, float y3, float texSrc3, float texDest3,
		float x4, float y4, float texSrc4, float texDest4
	);

	inline void vert(
		const Texture* texture,
		const float x1, const float y1, const Color c1, const Color cm1,
		const float x2, const float y2, const Color c2, const Color cm2,
		const float x3, const float y3, const Color c3, const Color cm3,
		const float x4, const float y4, const Color c4, const Color cm4
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
		const Texture* texture,
		const float x1, const float y1, const Color c1,
		const float x2, const float y2, const Color c2,
		const float x3, const float y3, const Color c3,
		const float x4, const float y4, const Color c4
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
		const Texture* texture,
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
		const Texture* texture,
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
	                 const float x1, const float y1, const Color c1,
	                 const float x2, const float y2, const Color c2,
	                 const float x3, const float y3, const Color c3,
	                 const float x4, const float y4, const Color c4
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
	          const T& v0, const Color c1,
	          const T& v1, const Color c2,
	          const T& v2, const Color c3,
	          const T& v3, const Color c4
	) {
		::Graphic::Draw::vert_monochromeMix(
			region->getData(), contextMixColor,
			v0.getX(), v0.getY(), region->u00(), region->v00(), c1,
			v1.getX(), v1.getY(), region->u10(), region->v10(), c2,
			v2.getX(), v2.getY(), region->u11(), region->v11(), c3,
			v3.getX(), v3.getY(), region->u01(), region->v01(), c4
		);
	}

	template <Concepts::Pos<float> T>
	void quad(const TextureRegion* region, const T& v0, const T& v1, const T& v2, const T& v3) {
		::Graphic::Draw::vert_monochromeAll(
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

	inline void reset() {
		contextStroke = 1.0f;
		color();
		mixColor();
		texture();
	}

	void fillSquare(const float x, const float y, const float radius, float ang){
		ang += 45.000f;

		vec2_0.setPolar(ang, radius / Math::SQRT2);

		vec2_1.set(x, y).add(vec2_0);
		vec2_2.set(x, y).add(vec2_0.rotateRT());

		vec2_3.set(x, y).add(vec2_0.rotateRT());
		vec2_4.set(x, y).add(vec2_0.rotateRT());

		Draw::quad(defaultTexture, vec2_1, vec2_2, vec2_3, vec2_4);
	}

	void poly(const float x, const float y, const int sides, const float radius, const float angle, const Color inner = contextColor, const Color exter = contextColor) {
		const float space  = 360.0f / static_cast<float>(sides);

		for(int i = 0; i < sides; i++) {
			const float a    = space * static_cast<float>(i) + angle;
			const float cos1 = Math::cosDeg(a);
			const float sin1 = Math::sinDeg(a);
			const float cos2 = Math::cosDeg(a + space);
			const float sin2 = Math::sinDeg(a + space);
			quad(
				defaultTexture,
				x, y, inner,
				x, y, inner,
				x + radius * cos2, y + radius * sin2, exter,
				x + radius * cos1, y + radius * sin1, exter
			);
		}
	}
}

