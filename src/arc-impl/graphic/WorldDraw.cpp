module Graphic.Draw.World;

import Core;
void Graphic::Draw::World::vert(GL::Texture* texture,
	float x1, float y1, float z1, float u1, float v1, const Color c1, const Color cm1,
	float x2, float y2, float z2, float u2, float v2, const Color c2, const Color cm2,
	float x3, float y3, float z3, float u3, float v3, const Color c3, const Color cm3,
	float x4, float y4, float z4, float u4, float v4, const Color c4, const Color cm4)
{
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

	Core::worldBatch->post(texture, vertices, VERT_LENGTH_WORLD, 0, VERT_LENGTH_WORLD);
}

void Graphic::Draw::World::vert_monochromeMix(const Texture* texture, const Color cm, float x1, float y1, float z1,
	float u1, float v1, const Color c1, float x2, float y2, float z2, float u2, float v2,
	const Color c2, float x3, float y3, float z3, float u3, float v3, const Color c3, float x4,
	float y4, float z4, float u4, float v4, const Color c4){
	
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

	Core::worldBatch->post(texture, vertices, VERT_LENGTH_WORLD, 0, VERT_LENGTH_WORLD);

}

void Graphic::Draw::World::vert_monochromeAll(const Texture* texture, const Color c, const Color cm, float x1, float y1,
	float z1, float u1, float v1, float x2, float y2, float z2, float u2, float v2, float x3,
	float y3, float z3, float u3, float v3, float x4, float y4, float z4,
	float u4, float v4){

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

	Core::worldBatch->post(texture, vertices, VERT_LENGTH_WORLD, 0, VERT_LENGTH_WORLD);

}

void Graphic::Draw::World::flush(){
	Core::worldBatch->flush();
}
