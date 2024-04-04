module Graphic.Draw;

import Core;

void Graphic::World::vert(float* vertices, const float x1, const float y1, const float z1, const float u1, const float v1, const Color c1, const Color cm1, const float x2,
                       const float y2, const float z2, const float u2, const float v2, const Color c2, const Color cm2, const float x3, const float y3, const float z3, const float u3, const float v3,
                       const Color c3,
                       const Color cm3, const float x4, const float y4, const float z4, const float u4, const float v4, const Color c4, const Color cm4){
	vertices[0  + 0 * VERT_GROUP_SIZE_WORLD] = x1;
	vertices[1  + 0 * VERT_GROUP_SIZE_WORLD] = y1;
	vertices[2  + 0 * VERT_GROUP_SIZE_WORLD] = z1;
	vertices[3  + 0 * VERT_GROUP_SIZE_WORLD] = u1;
	vertices[4  + 0 * VERT_GROUP_SIZE_WORLD] = v1;
	vertices[5  + 0 * VERT_GROUP_SIZE_WORLD] = cm1.r;
	vertices[6  + 0 * VERT_GROUP_SIZE_WORLD] = cm1.g;
	vertices[7  + 0 * VERT_GROUP_SIZE_WORLD] = cm1.b;
	vertices[8  + 0 * VERT_GROUP_SIZE_WORLD] = cm1.a;
	vertices[9  + 0 * VERT_GROUP_SIZE_WORLD] = c1.r;
	vertices[10 + 0 * VERT_GROUP_SIZE_WORLD] = c1.g;
	vertices[11 + 0 * VERT_GROUP_SIZE_WORLD] = c1.b;
	vertices[12 + 0 * VERT_GROUP_SIZE_WORLD] = c1.a;

	vertices[0  + 1 * VERT_GROUP_SIZE_WORLD] = x2;
	vertices[1  + 1 * VERT_GROUP_SIZE_WORLD] = y2;
	vertices[2  + 1 * VERT_GROUP_SIZE_WORLD] = z2;
	vertices[3  + 1 * VERT_GROUP_SIZE_WORLD] = u2;
	vertices[4  + 1 * VERT_GROUP_SIZE_WORLD] = v2;
	vertices[5  + 1 * VERT_GROUP_SIZE_WORLD] = cm2.r;
	vertices[6  + 1 * VERT_GROUP_SIZE_WORLD] = cm2.g;
	vertices[7  + 1 * VERT_GROUP_SIZE_WORLD] = cm2.b;
	vertices[8  + 1 * VERT_GROUP_SIZE_WORLD] = cm2.a;
	vertices[9  + 1 * VERT_GROUP_SIZE_WORLD] = c2.r;
	vertices[10 + 1 * VERT_GROUP_SIZE_WORLD] = c2.g;
	vertices[11 + 1 * VERT_GROUP_SIZE_WORLD] = c2.b;
	vertices[12 + 1 * VERT_GROUP_SIZE_WORLD] = c2.a;

	vertices[0  + 2 * VERT_GROUP_SIZE_WORLD] = x3;
	vertices[1  + 2 * VERT_GROUP_SIZE_WORLD] = y3;
	vertices[2  + 2 * VERT_GROUP_SIZE_WORLD] = z3;
	vertices[3  + 2 * VERT_GROUP_SIZE_WORLD] = u3;
	vertices[4  + 2 * VERT_GROUP_SIZE_WORLD] = v3;
	vertices[5  + 2 * VERT_GROUP_SIZE_WORLD] = cm3.r;
	vertices[6  + 2 * VERT_GROUP_SIZE_WORLD] = cm3.g;
	vertices[7  + 2 * VERT_GROUP_SIZE_WORLD] = cm3.b;
	vertices[8  + 2 * VERT_GROUP_SIZE_WORLD] = cm3.a;
	vertices[9  + 2 * VERT_GROUP_SIZE_WORLD] = c3.r;
	vertices[10 + 2 * VERT_GROUP_SIZE_WORLD] = c3.g;
	vertices[11 + 2 * VERT_GROUP_SIZE_WORLD] = c3.b;
	vertices[12 + 2 * VERT_GROUP_SIZE_WORLD] = c3.a;

	vertices[0  + 3 * VERT_GROUP_SIZE_WORLD] = x4;
	vertices[1  + 3 * VERT_GROUP_SIZE_WORLD] = y4;
	vertices[2  + 3 * VERT_GROUP_SIZE_WORLD] = z4;
	vertices[3  + 3 * VERT_GROUP_SIZE_WORLD] = u4;
	vertices[4  + 3 * VERT_GROUP_SIZE_WORLD] = v4;
	vertices[5  + 3 * VERT_GROUP_SIZE_WORLD] = cm4.r;
	vertices[6  + 3 * VERT_GROUP_SIZE_WORLD] = cm4.g;
	vertices[7  + 3 * VERT_GROUP_SIZE_WORLD] = cm4.b;
	vertices[8  + 3 * VERT_GROUP_SIZE_WORLD] = cm4.a;
	vertices[9  + 3 * VERT_GROUP_SIZE_WORLD] = c4.r;
	vertices[10 + 3 * VERT_GROUP_SIZE_WORLD] = c4.g;
	vertices[11 + 3 * VERT_GROUP_SIZE_WORLD] = c4.b;
	vertices[12 + 3 * VERT_GROUP_SIZE_WORLD] = c4.a;
}

void Graphic::World::vert_monochromeMix(float* vertices, const Color cm, const float x1, const float y1, const float z1, const float u1, const float v1,
                                     const Color c1, const float x2, const float y2, const float z2, const float u2, const float v2, const Color c2, const float x3, const float y3, const float z3, const float u3,
                                     const float v3, const Color c3, const float x4,
                                     const float y4, const float z4, const float u4, const float v4, const Color c4){
	vertices[0  + 0 * VERT_GROUP_SIZE_WORLD] = x1;
	vertices[1  + 0 * VERT_GROUP_SIZE_WORLD] = y1;
	vertices[2  + 0 * VERT_GROUP_SIZE_WORLD] = z1;
	vertices[3  + 0 * VERT_GROUP_SIZE_WORLD] = u1;
	vertices[4  + 0 * VERT_GROUP_SIZE_WORLD] = v1;
	vertices[5  + 0 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 0 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 0 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 0 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 0 * VERT_GROUP_SIZE_WORLD] = c1.r;
	vertices[10 + 0 * VERT_GROUP_SIZE_WORLD] = c1.g;
	vertices[11 + 0 * VERT_GROUP_SIZE_WORLD] = c1.b;
	vertices[12 + 0 * VERT_GROUP_SIZE_WORLD] = c1.a;

	vertices[0  + 1 * VERT_GROUP_SIZE_WORLD] = x2;
	vertices[1  + 1 * VERT_GROUP_SIZE_WORLD] = y2;
	vertices[2  + 1 * VERT_GROUP_SIZE_WORLD] = z2;
	vertices[3  + 1 * VERT_GROUP_SIZE_WORLD] = u2;
	vertices[4  + 1 * VERT_GROUP_SIZE_WORLD] = v2;
	vertices[5  + 1 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 1 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 1 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 1 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 1 * VERT_GROUP_SIZE_WORLD] = c2.r;
	vertices[10 + 1 * VERT_GROUP_SIZE_WORLD] = c2.g;
	vertices[11 + 1 * VERT_GROUP_SIZE_WORLD] = c2.b;
	vertices[12 + 1 * VERT_GROUP_SIZE_WORLD] = c2.a;

	vertices[0  + 2 * VERT_GROUP_SIZE_WORLD] = x3;
	vertices[1  + 2 * VERT_GROUP_SIZE_WORLD] = y3;
	vertices[2  + 2 * VERT_GROUP_SIZE_WORLD] = z3;
	vertices[3  + 2 * VERT_GROUP_SIZE_WORLD] = u3;
	vertices[4  + 2 * VERT_GROUP_SIZE_WORLD] = v3;
	vertices[5  + 2 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 2 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 2 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 2 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 2 * VERT_GROUP_SIZE_WORLD] = c3.r;
	vertices[10 + 2 * VERT_GROUP_SIZE_WORLD] = c3.g;
	vertices[11 + 2 * VERT_GROUP_SIZE_WORLD] = c3.b;
	vertices[12 + 2 * VERT_GROUP_SIZE_WORLD] = c3.a;

	vertices[0  + 3 * VERT_GROUP_SIZE_WORLD] = x4;
	vertices[1  + 3 * VERT_GROUP_SIZE_WORLD] = y4;
	vertices[2  + 3 * VERT_GROUP_SIZE_WORLD] = z4;
	vertices[3  + 3 * VERT_GROUP_SIZE_WORLD] = u4;
	vertices[4  + 3 * VERT_GROUP_SIZE_WORLD] = v4;
	vertices[5  + 3 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 3 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 3 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 3 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 3 * VERT_GROUP_SIZE_WORLD] = c4.r;
	vertices[10 + 3 * VERT_GROUP_SIZE_WORLD] = c4.g;
	vertices[11 + 3 * VERT_GROUP_SIZE_WORLD] = c4.b;
	vertices[12 + 3 * VERT_GROUP_SIZE_WORLD] = c4.a;
}

void Graphic::World::vert_monochromeAll(float* vertices, const Color c, const Color cm, const float x1, const float y1, const float z1, const float u1, const float v1,
                                     const float x2, const float y2, const float z2, const float u2, const float v2, const float x3, const float y3, const float z3, const float u3, const float v3, const float x4,
                                     const float y4, const float z4,
                                     const float u4, const float v4){

	vertices[0  + 0 * VERT_GROUP_SIZE_WORLD] = x1;
	vertices[1  + 0 * VERT_GROUP_SIZE_WORLD] = y1;
	vertices[2  + 0 * VERT_GROUP_SIZE_WORLD] = z1;
	vertices[3  + 0 * VERT_GROUP_SIZE_WORLD] = u1;
	vertices[4  + 0 * VERT_GROUP_SIZE_WORLD] = v1;
	vertices[5  + 0 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 0 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 0 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 0 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 0 * VERT_GROUP_SIZE_WORLD] = c.r;
	vertices[10 + 0 * VERT_GROUP_SIZE_WORLD] = c.g;
	vertices[11 + 0 * VERT_GROUP_SIZE_WORLD] = c.b;
	vertices[12 + 0 * VERT_GROUP_SIZE_WORLD] = c.a;

	vertices[0  + 1 * VERT_GROUP_SIZE_WORLD] = x2;
	vertices[1  + 1 * VERT_GROUP_SIZE_WORLD] = y2;
	vertices[2  + 1 * VERT_GROUP_SIZE_WORLD] = z2;
	vertices[3  + 1 * VERT_GROUP_SIZE_WORLD] = u2;
	vertices[4  + 1 * VERT_GROUP_SIZE_WORLD] = v2;
	vertices[5  + 1 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 1 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 1 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 1 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 1 * VERT_GROUP_SIZE_WORLD] = c.r;
	vertices[10 + 1 * VERT_GROUP_SIZE_WORLD] = c.g;
	vertices[11 + 1 * VERT_GROUP_SIZE_WORLD] = c.b;
	vertices[12 + 1 * VERT_GROUP_SIZE_WORLD] = c.a;

	vertices[0  + 2 * VERT_GROUP_SIZE_WORLD] = x3;
	vertices[1  + 2 * VERT_GROUP_SIZE_WORLD] = y3;
	vertices[2  + 2 * VERT_GROUP_SIZE_WORLD] = z3;
	vertices[3  + 2 * VERT_GROUP_SIZE_WORLD] = u3;
	vertices[4  + 2 * VERT_GROUP_SIZE_WORLD] = v3;
	vertices[5  + 2 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 2 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 2 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 2 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 2 * VERT_GROUP_SIZE_WORLD] = c.r;
	vertices[10 + 2 * VERT_GROUP_SIZE_WORLD] = c.g;
	vertices[11 + 2 * VERT_GROUP_SIZE_WORLD] = c.b;
	vertices[12 + 2 * VERT_GROUP_SIZE_WORLD] = c.a;

	vertices[0  + 3 * VERT_GROUP_SIZE_WORLD] = x4;
	vertices[1  + 3 * VERT_GROUP_SIZE_WORLD] = y4;
	vertices[2  + 3 * VERT_GROUP_SIZE_WORLD] = z4;
	vertices[3  + 3 * VERT_GROUP_SIZE_WORLD] = u4;
	vertices[4  + 3 * VERT_GROUP_SIZE_WORLD] = v4;
	vertices[5  + 3 * VERT_GROUP_SIZE_WORLD] = cm.r;
	vertices[6  + 3 * VERT_GROUP_SIZE_WORLD] = cm.g;
	vertices[7  + 3 * VERT_GROUP_SIZE_WORLD] = cm.b;
	vertices[8  + 3 * VERT_GROUP_SIZE_WORLD] = cm.a;
	vertices[9  + 3 * VERT_GROUP_SIZE_WORLD] = c.r;
	vertices[10 + 3 * VERT_GROUP_SIZE_WORLD] = c.g;
	vertices[11 + 3 * VERT_GROUP_SIZE_WORLD] = c.b;
	vertices[12 + 3 * VERT_GROUP_SIZE_WORLD] = c.a;
}

void Graphic::Overlay::vert(float* vertices, const float x1, const float y1, const float u1, const float v1, const Color c1, const Color cm1, const float x2, const float y2,
                         const float u2, const float v2, const Color c2, const Color cm2, const float x3, const float y3, const float u3, const float v3, const Color c3, const Color cm3, const float x4,
                         const float y4,
                         const float u4, const float v4, const Color c4, const Color cm4){
	vertices[0 + 0 * VERT_GROUP_SIZE_LAYOUT]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE_LAYOUT]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE_LAYOUT]  = u1;
		vertices[3 + 0 * VERT_GROUP_SIZE_LAYOUT]  = v1;
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
		vertices[2 + 1 * VERT_GROUP_SIZE_LAYOUT]  = u2;
		vertices[3 + 1 * VERT_GROUP_SIZE_LAYOUT]  = v2;
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
		vertices[2 + 2 * VERT_GROUP_SIZE_LAYOUT]  = u3;
		vertices[3 + 2 * VERT_GROUP_SIZE_LAYOUT]  = v3;
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
		vertices[2 + 3 * VERT_GROUP_SIZE_LAYOUT]  = u4;
		vertices[3 + 3 * VERT_GROUP_SIZE_LAYOUT]  = v4;
		vertices[4 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.r;
		vertices[5 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.g;
		vertices[6 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.b;
		vertices[7 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm4.a;
		vertices[8 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.r;
		vertices[9 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.g;
		vertices[10 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.b;
		vertices[11 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.a;
}

void Graphic::Overlay::vert_monochromeMix(float* vertices, const Color cm, const float x1, const float y1, const float u1, const float v1, const Color c1,
                                       const float x2, const float y2, const float u2, const float v2, const Color c2, const float x3, const float y3, const float u3, const float v3, const Color c3, const float x4,
                                       const float y4, const float u4, const float v4,
                                       const Color c4){
	vertices[0 + 0 * VERT_GROUP_SIZE_LAYOUT]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE_LAYOUT]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE_LAYOUT]  = u1;
		vertices[3 + 0 * VERT_GROUP_SIZE_LAYOUT]  = v1;
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
		vertices[2 + 1 * VERT_GROUP_SIZE_LAYOUT]  = u2;
		vertices[3 + 1 * VERT_GROUP_SIZE_LAYOUT]  = v2;
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
		vertices[2 + 2 * VERT_GROUP_SIZE_LAYOUT]  = u3;
		vertices[3 + 2 * VERT_GROUP_SIZE_LAYOUT]  = v3;
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
		vertices[2 + 3 * VERT_GROUP_SIZE_LAYOUT]  = u4;
		vertices[3 + 3 * VERT_GROUP_SIZE_LAYOUT]  = v4;
		vertices[4 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.r;
		vertices[9 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c4.g;
		vertices[10 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.b;
		vertices[11 + 3 * VERT_GROUP_SIZE_LAYOUT] = c4.a;
}

void Graphic::Overlay::vert_monochromeAll(float* vertices, const Color c, const Color cm, const float x1, const float y1, const float u1, const float v1,
                                       const float x2, const float y2, const float u2, const float v2, const float x3, const float y3, const float u3, const float v3, const float x4, const float y4, const float u4,
                                       const float v4){
	vertices[0 + 0 * VERT_GROUP_SIZE_LAYOUT]  = x1;
		vertices[1 + 0 * VERT_GROUP_SIZE_LAYOUT]  = y1;
		vertices[2 + 0 * VERT_GROUP_SIZE_LAYOUT]  = u1;
		vertices[3 + 0 * VERT_GROUP_SIZE_LAYOUT]  = v1;
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
		vertices[2 + 1 * VERT_GROUP_SIZE_LAYOUT]  = u2;
		vertices[3 + 1 * VERT_GROUP_SIZE_LAYOUT]  = v2;
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
		vertices[2 + 2 * VERT_GROUP_SIZE_LAYOUT]  = u3;
		vertices[3 + 2 * VERT_GROUP_SIZE_LAYOUT]  = v3;
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
		vertices[2 + 3 * VERT_GROUP_SIZE_LAYOUT]  = u4;
		vertices[3 + 3 * VERT_GROUP_SIZE_LAYOUT]  = v4;
		vertices[4 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.r;
		vertices[5 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.g;
		vertices[6 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.b;
		vertices[7 + 3 * VERT_GROUP_SIZE_LAYOUT]  = cm.a;
		vertices[8 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c.r;
		vertices[9 + 3 * VERT_GROUP_SIZE_LAYOUT]  = c.g;
		vertices[10 + 3 * VERT_GROUP_SIZE_LAYOUT] = c.b;
		vertices[11 + 3 * VERT_GROUP_SIZE_LAYOUT] = c.a;
}

Graphic::BatchPtr& Graphic::getBatch(BatchPtr Core::BatchGroup::* batchPtr){
	return Core::batchGroup.*batchPtr;
}
