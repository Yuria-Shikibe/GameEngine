module Graphic.Draw;

import Core;
import Core.Batch;

namespace Graphic {
	const Matrix3D& Draw::getPorj() {
		return *Core::overlayBatch->getProjection();
	}

	void Draw::beginPorj(const Matrix3D& mat) {
		Core::overlayBatch->beginTempProjection(mat);
	}

	void Draw::endPorj() {
		Core::overlayBatch->endTempProjection();
	}

	void Draw::blend(const Blending& blending) {
		Core::overlayBatch->switchBlending(blending);
	}

	void Draw::flush() {
		Core::overlayBatch->flush();
	}

	void Draw::shader(Shader* shader, const bool flushContext) {
		if(flushContext) flush();

		Core::overlayBatch->setCustomShader(shader);
	}

	void Draw::shader(const bool flushContext) {
		Core::overlayBatch->clearCustomShader(flushContext);
	}

	void Draw::vert(const Texture2D* texture, const float x1, const float y1, const float texSrc1, const float texDest1,
		const Color& c1, const Color& cm1, const float x2, const float y2, const float texSrc2, const float texDest2,
		const Color& c2, const Color& cm2, const float x3, const float y3, const float texSrc3, const float texDest3,
		const Color& c3, const Color& cm3, const float x4, const float y4,
		const float texSrc4, const float texDest4, const Color& c4, const Color& cm4) {
		vertices[0 + 0 * VERT_GROUP_SIZE_LAYOUT]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE_LAYOUT]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE_LAYOUT]  = texSrc1;
		vertices[3 + 0 * VERT_GROUP_SIZE_LAYOUT]  = texDest1;
		vertices[4 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm1.r;
		vertices[5 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm1.g;
		vertices[6 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm1.b;
		vertices[7 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm1.a;
		vertices[8 + 0 * VERT_GROUP_SIZE_LAYOUT]  = c1.r;
		vertices[9 + 0 * VERT_GROUP_SIZE_LAYOUT]  = c1.g;
		vertices[10 + 0 * VERT_GROUP_SIZE_LAYOUT] = c1.b;
		vertices[11 + 0 * VERT_GROUP_SIZE_LAYOUT] = c1.a;

		vertices[0 + 1 * VERT_GROUP_SIZE_LAYOUT]  = x2;
		vertices[1 + 1 * VERT_GROUP_SIZE_LAYOUT]  = y2;
		vertices[2 + 1 * VERT_GROUP_SIZE_LAYOUT]  = texSrc2;
		vertices[3 + 1 * VERT_GROUP_SIZE_LAYOUT]  = texDest2;
		vertices[4 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm2.r;
		vertices[5 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm2.g;
		vertices[6 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm2.b;
		vertices[7 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm2.a;
		vertices[8 + 1 * VERT_GROUP_SIZE_LAYOUT]  = c2.r;
		vertices[9 + 1 * VERT_GROUP_SIZE_LAYOUT]  = c2.g;
		vertices[10 + 1 * VERT_GROUP_SIZE_LAYOUT] = c2.b;
		vertices[11 + 1 * VERT_GROUP_SIZE_LAYOUT] = c2.a;

		vertices[0 + 2 * VERT_GROUP_SIZE_LAYOUT]  = x3;
		vertices[1 + 2 * VERT_GROUP_SIZE_LAYOUT]  = y3;
		vertices[2 + 2 * VERT_GROUP_SIZE_LAYOUT]  = texSrc3;
		vertices[3 + 2 * VERT_GROUP_SIZE_LAYOUT]  = texDest3;
		vertices[4 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm3.r;
		vertices[5 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm3.g;
		vertices[6 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm3.b;
		vertices[7 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm3.a;
		vertices[8 + 2 * VERT_GROUP_SIZE_LAYOUT]  = c3.r;
		vertices[9 + 2 * VERT_GROUP_SIZE_LAYOUT]  = c3.g;
		vertices[10 + 2 * VERT_GROUP_SIZE_LAYOUT] = c3.b;
		vertices[11 + 2 * VERT_GROUP_SIZE_LAYOUT] = c3.a;

		vertices[0 + 3 * VERT_GROUP_SIZE_LAYOUT]  = x4;
		vertices[1 + 3 * VERT_GROUP_SIZE_LAYOUT]  = y4;
		vertices[2 + 3 * VERT_GROUP_SIZE_LAYOUT]  = texSrc4;
		vertices[3 + 3 * VERT_GROUP_SIZE_LAYOUT]  = texDest4;
		vertices[4 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.r;
		vertices[5 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.g;
		vertices[6 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.b;
		vertices[7 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.a;
		vertices[8 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.r;
		vertices[9 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.g;
		vertices[10 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.b;
		vertices[11 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.a;

		Core::overlayBatch->post(texture, vertices, VERT_LENGTH_STD, 0, VERT_LENGTH_STD);
	}

	void Draw::vert_monochromeMix(const Texture2D* texture, const Color& cm, const float x1, const float y1,
		const float texSrc1, const float texDest1, const Color& c1, const float x2, const float y2, const float texSrc2,
		const float texDest2, const Color& c2, const float x3, const float y3, const float texSrc3, const float texDest3,
		const Color& c3, const float x4, const float y4, const float texSrc4, const float texDest4,
		const Color& c4) {
		vertices[0 + 0 * VERT_GROUP_SIZE_LAYOUT]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE_LAYOUT]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE_LAYOUT]  = texSrc1;
		vertices[3 + 0 * VERT_GROUP_SIZE_LAYOUT]  = texDest1;
		vertices[4 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 0 * VERT_GROUP_SIZE_LAYOUT]  = c1.r;
		vertices[9 + 0 * VERT_GROUP_SIZE_LAYOUT]  = c1.g;
		vertices[10 + 0 * VERT_GROUP_SIZE_LAYOUT] = c1.b;
		vertices[11 + 0 * VERT_GROUP_SIZE_LAYOUT] = c1.a;

		vertices[0 + 1 * VERT_GROUP_SIZE_LAYOUT]  = x2;
		vertices[1 + 1 * VERT_GROUP_SIZE_LAYOUT]  = y2;
		vertices[2 + 1 * VERT_GROUP_SIZE_LAYOUT]  = texSrc2;
		vertices[3 + 1 * VERT_GROUP_SIZE_LAYOUT]  = texDest2;
		vertices[4 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 1 * VERT_GROUP_SIZE_LAYOUT]  = c2.r;
		vertices[9 + 1 * VERT_GROUP_SIZE_LAYOUT]  = c2.g;
		vertices[10 + 1 * VERT_GROUP_SIZE_LAYOUT] = c2.b;
		vertices[11 + 1 * VERT_GROUP_SIZE_LAYOUT] = c2.a;

		vertices[0 + 2 * VERT_GROUP_SIZE_LAYOUT]  = x3;
		vertices[1 + 2 * VERT_GROUP_SIZE_LAYOUT]  = y3;
		vertices[2 + 2 * VERT_GROUP_SIZE_LAYOUT]  = texSrc3;
		vertices[3 + 2 * VERT_GROUP_SIZE_LAYOUT]  = texDest3;
		vertices[4 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 2 * VERT_GROUP_SIZE_LAYOUT]  = c3.r;
		vertices[9 + 2 * VERT_GROUP_SIZE_LAYOUT]  = c3.g;
		vertices[10 + 2 * VERT_GROUP_SIZE_LAYOUT] = c3.b;
		vertices[11 + 2 * VERT_GROUP_SIZE_LAYOUT] = c3.a;

		vertices[0 + 3 * VERT_GROUP_SIZE_LAYOUT]  = x4;
		vertices[1 + 3 * VERT_GROUP_SIZE_LAYOUT]  = y4;
		vertices[2 + 3 * VERT_GROUP_SIZE_LAYOUT]  = texSrc4;
		vertices[3 + 3 * VERT_GROUP_SIZE_LAYOUT]  = texDest4;
		vertices[4 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.r;
		vertices[9 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.g;
		vertices[10 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.b;
		vertices[11 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.a;

		Core::overlayBatch->post(texture, vertices, VERT_LENGTH_STD, 0, VERT_LENGTH_STD);
	}

	void Draw::vert_monochromeAll(const Texture2D* texture, const Color& c, const Color& cm, const float x1, const float y1,
		const float texSrc1, const float texDest1, const float x2, const float y2, const float texSrc2,
		const float texDest2, const float x3, const float y3, const float texSrc3, const float texDest3, const float x4,
		const float y4, const float texSrc4, const float texDest4) {
		vertices[0 + 0 * VERT_GROUP_SIZE_LAYOUT]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE_LAYOUT]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE_LAYOUT]  = texSrc1;
		vertices[3 + 0 * VERT_GROUP_SIZE_LAYOUT]  = texDest1;
		vertices[4 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 0 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 0 * VERT_GROUP_SIZE_LAYOUT]  = c.r;
		vertices[9 + 0 * VERT_GROUP_SIZE_LAYOUT]  = c.g;
		vertices[10 + 0 * VERT_GROUP_SIZE_LAYOUT] = c.b;
		vertices[11 + 0 * VERT_GROUP_SIZE_LAYOUT] = c.a;

		vertices[0 + 1 * VERT_GROUP_SIZE_LAYOUT]  = x2;
		vertices[1 + 1 * VERT_GROUP_SIZE_LAYOUT]  = y2;
		vertices[2 + 1 * VERT_GROUP_SIZE_LAYOUT]  = texSrc2;
		vertices[3 + 1 * VERT_GROUP_SIZE_LAYOUT]  = texDest2;
		vertices[4 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 1 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 1 * VERT_GROUP_SIZE_LAYOUT]  = c.r;
		vertices[9 + 1 * VERT_GROUP_SIZE_LAYOUT]  = c.g;
		vertices[10 + 1 * VERT_GROUP_SIZE_LAYOUT] = c.b;
		vertices[11 + 1 * VERT_GROUP_SIZE_LAYOUT] = c.a;

		vertices[0 + 2 * VERT_GROUP_SIZE_LAYOUT]  = x3;
		vertices[1 + 2 * VERT_GROUP_SIZE_LAYOUT]  = y3;
		vertices[2 + 2 * VERT_GROUP_SIZE_LAYOUT]  = texSrc3;
		vertices[3 + 2 * VERT_GROUP_SIZE_LAYOUT]  = texDest3;
		vertices[4 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 2 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 2 * VERT_GROUP_SIZE_LAYOUT]  = c.r;
		vertices[9 + 2 * VERT_GROUP_SIZE_LAYOUT]  = c.g;
		vertices[10 + 2 * VERT_GROUP_SIZE_LAYOUT] = c.b;
		vertices[11 + 2 * VERT_GROUP_SIZE_LAYOUT] = c.a;

		vertices[0 + 3 * VERT_GROUP_SIZE_LAYOUT]  = x4;
		vertices[1 + 3 * VERT_GROUP_SIZE_LAYOUT]  = y4;
		vertices[2 + 3 * VERT_GROUP_SIZE_LAYOUT]  = texSrc4;
		vertices[3 + 3 * VERT_GROUP_SIZE_LAYOUT]  = texDest4;
		vertices[4 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c.r;
		vertices[9 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c.g;
		vertices[10 + 3 * VERT_GROUP_SIZE_LAYOUT] = c.b;
		vertices[11 + 3 * VERT_GROUP_SIZE_LAYOUT] = c.a;

		Core::overlayBatch->post(texture, vertices, VERT_LENGTH_STD, 0, VERT_LENGTH_STD);
	}
}
