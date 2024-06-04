module GL.Texture.TextureNineRegion;

import Graphic.Draw;

using namespace Graphic;

void GL::TextureNineRegion::render_RelativeExter(const float x, const float y, const float width, const float height) const {
	static constexpr auto EdgeScale = Align::Scale::bounded;

	auto botLftSize = edge.bot_lft();
	auto topRitSize = edge.top_rit();

	const auto cornerSize = botLftSize + topRitSize;
	const auto scale = Align::embedTo(EdgeScale, cornerSize, {width, height}) / cornerSize;

	botLftSize *= scale;
	topRitSize *= scale;

	Geom::Vec2 anchorBotLft{botLftSize};
	Geom::Vec2 anchorTopRit{Geom::Vec2{width, height} - topRitSize};

	anchorBotLft.add(x, y);
	anchorTopRit.add(x, y);

	[[assume(anchorBotLft.x <= anchorTopRit.x)]];
	[[assume(anchorBotLft.y <= anchorTopRit.y)]];

	[[assume(x <= anchorBotLft.x)]];
	[[assume(y <= anchorBotLft.y)]];

	[[assume(anchorTopRit.x <= x + width)]];
	[[assume(anchorTopRit.y <= y + height)]];

	const Rect center{anchorBotLft, anchorTopRit};

	const Rect bot_lft{{x, y}, center.vert_00()};
	const Rect bot_rit{center.vert_10(), {x + width, y}};
	const Rect bot{bot_lft.vert_10(), bot_rit.vert_01()};

	const Rect lft{bot_lft.vert_01(), center.vert_01()};
	const Rect rit{bot_rit.vert_11(), center.vert_11()};

	const Rect top_rit{center.vert_11(), {x + width, y + height}};
	const Rect top_lft{center.vert_01(), {x, y + height}};
	const Rect top{top_lft.vert_10(), top_rit.vert_01()};

	if(centerScale != Align::Scale::fill){
		const auto centerSize = Align::embedTo(centerScale, innerSize, center.getSize());
		auto [cx, cy] = Align::getOffsetOf(Align::Layout::center, centerSize, center);

		Draw::Overlay::Fill::rectOrtho(regions[ID_center], cx, cy, centerSize.x, centerSize.y);
	}else{
		Draw::Overlay::Fill::rectOrtho(regions[ID_center], center);
	}

	Draw::Overlay::Fill::rectOrtho(regions[ID_right], rit);
	Draw::Overlay::Fill::rectOrtho(regions[ID_top], top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_left], lft);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottom], bot);

	Draw::Overlay::Fill::rectOrtho(regions[ID_topRight], top_rit);
	Draw::Overlay::Fill::rectOrtho(regions[ID_topLeft], top_lft);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomLeft], bot_lft);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomRight], bot_rit);
}

void GL::TextureNineRegion::render_RelativeInner(const float x, const float y, const float width,
		const float height) const {
	Draw::Overlay::Fill::rectOrtho(regions[ID_center], x, y, width, height);

	Draw::Overlay::Fill::rectOrtho(regions[ID_right], x + width, y, edge.right, height);
	Draw::Overlay::Fill::rectOrtho(regions[ID_top], x, y + height, width, edge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_left], x - edge.left, y, edge.left, height);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottom], x, y - edge.bottom, width, edge.bottom);

	Draw::Overlay::Fill::rectOrtho(regions[ID_topRight], x + width, y + height, edge.right, edge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_topLeft], x - edge.left, y + height, edge.left, edge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomLeft], x - edge.left, y - edge.bottom, edge.left, edge.bottom);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomRight], x + width, y - edge.bottom, edge.right, edge.bottom);
}