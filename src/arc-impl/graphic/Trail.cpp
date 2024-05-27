module Graphic.Trail;

import Graphic.Draw;
import Core.BatchGroup;

void Graphic::Trail::DefDraw::operator()(const Geom::Vec2 v1, const Geom::Vec2 v2, const Geom::Vec2 v3, const Geom::Vec2 v4) const{
	Draw::World::Fill::quad(Draw::World::getContextTexture(), v1, src, v2, src, v3, src, v4, src);
}

void Graphic::Trail::DefDraw_WithLerp::operator()(const Geom::Vec2 v1, const Geom::Vec2 v2, const Geom::Vec2 v3, const Geom::Vec2 v4, const Math::Progress p1,
                                             const Math::Progress p2) const{
	const Graphic::Color c1 = Graphic::Color::createLerp(p1, dst, src);
	const Graphic::Color c2 = Graphic::Color::createLerp(p2, dst, src);
	Draw::World::Fill::quad(Draw::World::getContextTexture(), v1, c1, v2, c1, v3, c2, v4, c2);

}
