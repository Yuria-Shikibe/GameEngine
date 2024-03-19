//
// Created by Matrix on 2024/3/16.
//

export module Graphic.Draw.World;

import GL.Constants;
import GL;
import GL.Texture;
import GL.Texture.TextureRegion;
import GL.Texture.TextureRegionRect;

import Graphic.Color;

import Math;
import Geom.Vector2D;

using namespace GL;

constexpr float near = 1;
constexpr float far = 300;

export namespace Graphic::Draw::World{
	inline float vertices[VERT_LENGTH_WORLD]{};

	constexpr float getNormalizedDepth(const float z){
		return (1 / z - 1 / near) / (1 / far - 1 / near);
	}

	inline Color contextColor = Colors::WHITE;
	inline Color contextMixColor = Colors::CLEAR;

	inline const TextureRegion* contextTexture = nullptr;
	inline const TextureRegion* defaultTexture = nullptr;

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

	void endPorj();

	void color(const Color c1, const Color c2, const float t) {
		contextColor.lerp(t, c1, c2);
	}

	inline void texture(const TextureRegion* texture = defaultTexture) {
		contextTexture = texture;
	}

	inline void defTexture(const TextureRegion* texture) {
		defaultTexture = texture;
	}

	void vert(
		GL::Texture* texture,
		float x1, float y1, float z1, float u1, float v1, Color c1, Color cm1,
		float x2, float y2, float z2, float u2, float v2, Color c2, Color cm2,
		float x3, float y3, float z3, float u3, float v3, Color c3, Color cm3,
		float x4, float y4, float z4, float u4, float v4, Color c4, Color cm4
	);

	void vert_monochromeMix(
		const Texture* texture, Color cm,
		float x1, float y1, float z1, float u1, float v1, Color c1,
		float x2, float y2, float z2, float u2, float v2, Color c2,
		float x3, float y3, float z3, float u3, float v3, Color c3,
		float x4, float y4, float z4, float u4, float v4, Color c4
	);

	void vert_monochromeAll(
		const Texture* texture, Color c, Color cm,
		float x1, float y1, float z1, float u1, float v1,
		float x2, float y2, float z2, float u2, float v2,
		float x3, float y3, float z3, float u3, float v3,
		float x4, float y4, float z4, float u4, float v4
	);

	inline void rect(const TextureRegionRect* region,
					 const float x, const float y, const float z,
					 const float ang
	) {
		const float sin = Math::sinDeg(ang);
		const float cos = Math::cosDeg(ang);
		const float w1  = cos * region->getWidth() * 0.5f;
		const float h1  = sin * region->getWidth() * 0.5f;

		const float w2 = -sin * region->getHeight() * 0.5f;
		const float h2 = cos * region->getHeight() * 0.5f;

		const float norZ = getNormalizedDepth(z);

		vert_monochromeAll(
			region->getData(), contextColor, contextMixColor,
			x - w1 - w2, y - h1 - h2, norZ, region->u00(), region->v00(),
			x + w1 - w2, y + h1 - h2, norZ, region->u10(), region->v10(),
			x + w1 + w2, y + h1 + h2, norZ, region->u11(), region->v11(),
			x - w1 + w2, y - h1 + h2, norZ, region->u01(), region->v01()
		);
	}

	void flush();
}
