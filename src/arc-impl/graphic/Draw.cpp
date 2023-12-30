module Graphic.Draw;

import Core;
import Core.Batch;

import GL.Buffer.IndexBuffer;

namespace Graphic {
	const Matrix3D& Draw::getPorj() {
		return *Core::batch->getProjection();
	}

	void Draw::beginPorj(const Matrix3D& mat) {
		Core::batch->beginProjection(mat);
	}

	void Draw::endPorj() {
		Core::batch->endProjection();
	}

	void Draw::blend(const Blending& blending) {
		Core::batch->switchBlending(blending);
	}

	void Draw::flush() {
		Core::batch->flush();
	}

	void Draw::shader(Shader* shader, const bool flushContext) {
		if(flushContext) flush();

		Core::batch->setCustomShader(shader);
	}

	void Draw::shader(const bool flushContext) {
		Core::batch->clearCustomShader(flushContext);
	}

	void Draw::vert(const Texture2D* texture, const float x1, const float y1, const float texSrc1, const float texDest1,
		const Color& c1, const Color& cm1, const float x2, const float y2, const float texSrc2, const float texDest2,
		const Color& c2, const Color& cm2, const float x3, const float y3, const float texSrc3, const float texDest3,
		const Color& c3, const Color& cm3, const float x4, const float y4,
		const float texSrc4, const float texDest4, const Color& c4, const Color& cm4) {
		vertices[0 + 0 * VERT_GROUP_SIZE]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE]  = texSrc1;
		vertices[3 + 0 * VERT_GROUP_SIZE]  = texDest1;
		vertices[4 + 0 * VERT_GROUP_SIZE]  = cm1.r;
		vertices[5 + 0 * VERT_GROUP_SIZE]  = cm1.g;
		vertices[6 + 0 * VERT_GROUP_SIZE]  = cm1.b;
		vertices[7 + 0 * VERT_GROUP_SIZE]  = cm1.a;
		vertices[8 + 0 * VERT_GROUP_SIZE]  = c1.r;
		vertices[9 + 0 * VERT_GROUP_SIZE]  = c1.g;
		vertices[10 + 0 * VERT_GROUP_SIZE] = c1.b;
		vertices[11 + 0 * VERT_GROUP_SIZE] = c1.a;

		vertices[0 + 1 * VERT_GROUP_SIZE]  = x2;
		vertices[1 + 1 * VERT_GROUP_SIZE]  = y2;
		vertices[2 + 1 * VERT_GROUP_SIZE]  = texSrc2;
		vertices[3 + 1 * VERT_GROUP_SIZE]  = texDest2;
		vertices[4 + 1 * VERT_GROUP_SIZE]  = cm2.r;
		vertices[5 + 1 * VERT_GROUP_SIZE]  = cm2.g;
		vertices[6 + 1 * VERT_GROUP_SIZE]  = cm2.b;
		vertices[7 + 1 * VERT_GROUP_SIZE]  = cm2.a;
		vertices[8 + 1 * VERT_GROUP_SIZE]  = c2.r;
		vertices[9 + 1 * VERT_GROUP_SIZE]  = c2.g;
		vertices[10 + 1 * VERT_GROUP_SIZE] = c2.b;
		vertices[11 + 1 * VERT_GROUP_SIZE] = c2.a;

		vertices[0 + 2 * VERT_GROUP_SIZE]  = x3;
		vertices[1 + 2 * VERT_GROUP_SIZE]  = y3;
		vertices[2 + 2 * VERT_GROUP_SIZE]  = texSrc3;
		vertices[3 + 2 * VERT_GROUP_SIZE]  = texDest3;
		vertices[4 + 2 * VERT_GROUP_SIZE]  = cm3.r;
		vertices[5 + 2 * VERT_GROUP_SIZE]  = cm3.g;
		vertices[6 + 2 * VERT_GROUP_SIZE]  = cm3.b;
		vertices[7 + 2 * VERT_GROUP_SIZE]  = cm3.a;
		vertices[8 + 2 * VERT_GROUP_SIZE]  = c3.r;
		vertices[9 + 2 * VERT_GROUP_SIZE]  = c3.g;
		vertices[10 + 2 * VERT_GROUP_SIZE] = c3.b;
		vertices[11 + 2 * VERT_GROUP_SIZE] = c3.a;

		vertices[0 + 3 * VERT_GROUP_SIZE]  = x4;
		vertices[1 + 3 * VERT_GROUP_SIZE]  = y4;
		vertices[2 + 3 * VERT_GROUP_SIZE]  = texSrc4;
		vertices[3 + 3 * VERT_GROUP_SIZE]  = texDest4;
		vertices[4 + 3 * VERT_GROUP_SIZE]  = cm4.r;
		vertices[5 + 3 * VERT_GROUP_SIZE]  = cm4.g;
		vertices[6 + 3 * VERT_GROUP_SIZE]  = cm4.b;
		vertices[7 + 3 * VERT_GROUP_SIZE]  = cm4.a;
		vertices[8 + 3 * VERT_GROUP_SIZE]  = c4.r;
		vertices[9 + 3 * VERT_GROUP_SIZE]  = c4.g;
		vertices[10 + 3 * VERT_GROUP_SIZE] = c4.b;
		vertices[11 + 3 * VERT_GROUP_SIZE] = c4.a;

		Core::batch->post(texture, vertices, VERT_LENGTH_STD, 0, VERT_LENGTH_STD);
	}

	void Draw::vert_monochromeMix(const Texture2D* texture, const Color& cm, const float x1, const float y1,
		const float texSrc1, const float texDest1, const Color& c1, const float x2, const float y2, const float texSrc2,
		const float texDest2, const Color& c2, const float x3, const float y3, const float texSrc3, const float texDest3,
		const Color& c3, const float x4, const float y4, const float texSrc4, const float texDest4,
		const Color& c4) {
		vertices[0 + 0 * VERT_GROUP_SIZE]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE]  = texSrc1;
		vertices[3 + 0 * VERT_GROUP_SIZE]  = texDest1;
		vertices[4 + 0 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 0 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 0 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 0 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 0 * VERT_GROUP_SIZE]  = c1.r;
		vertices[9 + 0 * VERT_GROUP_SIZE]  = c1.g;
		vertices[10 + 0 * VERT_GROUP_SIZE] = c1.b;
		vertices[11 + 0 * VERT_GROUP_SIZE] = c1.a;

		vertices[0 + 1 * VERT_GROUP_SIZE]  = x2;
		vertices[1 + 1 * VERT_GROUP_SIZE]  = y2;
		vertices[2 + 1 * VERT_GROUP_SIZE]  = texSrc2;
		vertices[3 + 1 * VERT_GROUP_SIZE]  = texDest2;
		vertices[4 + 1 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 1 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 1 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 1 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 1 * VERT_GROUP_SIZE]  = c2.r;
		vertices[9 + 1 * VERT_GROUP_SIZE]  = c2.g;
		vertices[10 + 1 * VERT_GROUP_SIZE] = c2.b;
		vertices[11 + 1 * VERT_GROUP_SIZE] = c2.a;

		vertices[0 + 2 * VERT_GROUP_SIZE]  = x3;
		vertices[1 + 2 * VERT_GROUP_SIZE]  = y3;
		vertices[2 + 2 * VERT_GROUP_SIZE]  = texSrc3;
		vertices[3 + 2 * VERT_GROUP_SIZE]  = texDest3;
		vertices[4 + 2 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 2 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 2 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 2 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 2 * VERT_GROUP_SIZE]  = c3.r;
		vertices[9 + 2 * VERT_GROUP_SIZE]  = c3.g;
		vertices[10 + 2 * VERT_GROUP_SIZE] = c3.b;
		vertices[11 + 2 * VERT_GROUP_SIZE] = c3.a;

		vertices[0 + 3 * VERT_GROUP_SIZE]  = x4;
		vertices[1 + 3 * VERT_GROUP_SIZE]  = y4;
		vertices[2 + 3 * VERT_GROUP_SIZE]  = texSrc4;
		vertices[3 + 3 * VERT_GROUP_SIZE]  = texDest4;
		vertices[4 + 3 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 3 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 3 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 3 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 3 * VERT_GROUP_SIZE]  = c4.r;
		vertices[9 + 3 * VERT_GROUP_SIZE]  = c4.g;
		vertices[10 + 3 * VERT_GROUP_SIZE] = c4.b;
		vertices[11 + 3 * VERT_GROUP_SIZE] = c4.a;

		Core::batch->post(texture, vertices, VERT_LENGTH_STD, 0, VERT_LENGTH_STD);
	}

	void Draw::vert_monochromeAll(const Texture2D* texture, const Color& c, const Color& cm, const float x1, const float y1,
		const float texSrc1, const float texDest1, const float x2, const float y2, const float texSrc2,
		const float texDest2, const float x3, const float y3, const float texSrc3, const float texDest3, const float x4,
		const float y4, const float texSrc4, const float texDest4) {
		vertices[0 + 0 * VERT_GROUP_SIZE]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE]  = texSrc1;
		vertices[3 + 0 * VERT_GROUP_SIZE]  = texDest1;
		vertices[4 + 0 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 0 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 0 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 0 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 0 * VERT_GROUP_SIZE]  = c.r;
		vertices[9 + 0 * VERT_GROUP_SIZE]  = c.g;
		vertices[10 + 0 * VERT_GROUP_SIZE] = c.b;
		vertices[11 + 0 * VERT_GROUP_SIZE] = c.a;

		vertices[0 + 1 * VERT_GROUP_SIZE]  = x2;
		vertices[1 + 1 * VERT_GROUP_SIZE]  = y2;
		vertices[2 + 1 * VERT_GROUP_SIZE]  = texSrc2;
		vertices[3 + 1 * VERT_GROUP_SIZE]  = texDest2;
		vertices[4 + 1 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 1 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 1 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 1 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 1 * VERT_GROUP_SIZE]  = c.r;
		vertices[9 + 1 * VERT_GROUP_SIZE]  = c.g;
		vertices[10 + 1 * VERT_GROUP_SIZE] = c.b;
		vertices[11 + 1 * VERT_GROUP_SIZE] = c.a;

		vertices[0 + 2 * VERT_GROUP_SIZE]  = x3;
		vertices[1 + 2 * VERT_GROUP_SIZE]  = y3;
		vertices[2 + 2 * VERT_GROUP_SIZE]  = texSrc3;
		vertices[3 + 2 * VERT_GROUP_SIZE]  = texDest3;
		vertices[4 + 2 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 2 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 2 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 2 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 2 * VERT_GROUP_SIZE]  = c.r;
		vertices[9 + 2 * VERT_GROUP_SIZE]  = c.g;
		vertices[10 + 2 * VERT_GROUP_SIZE] = c.b;
		vertices[11 + 2 * VERT_GROUP_SIZE] = c.a;

		vertices[0 + 3 * VERT_GROUP_SIZE]  = x4;
		vertices[1 + 3 * VERT_GROUP_SIZE]  = y4;
		vertices[2 + 3 * VERT_GROUP_SIZE]  = texSrc4;
		vertices[3 + 3 * VERT_GROUP_SIZE]  = texDest4;
		vertices[4 + 3 * VERT_GROUP_SIZE]  = cm.r;
		vertices[5 + 3 * VERT_GROUP_SIZE]  = cm.g;
		vertices[6 + 3 * VERT_GROUP_SIZE]  = cm.b;
		vertices[7 + 3 * VERT_GROUP_SIZE]  = cm.a;
		vertices[8 + 3 * VERT_GROUP_SIZE]  = c.r;
		vertices[9 + 3 * VERT_GROUP_SIZE]  = c.g;
		vertices[10 + 3 * VERT_GROUP_SIZE] = c.b;
		vertices[11 + 3 * VERT_GROUP_SIZE] = c.a;

		Core::batch->post(texture, vertices, VERT_LENGTH_STD, 0, VERT_LENGTH_STD);
	}
}
