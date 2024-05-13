//
// Created by Matrix on 2024/4/21.
//

export module Game.Chamber.Util;

export import Graphic.Pixmap;
export import Game.Chamber;
export import Game.Chamber.Frame;
import Geom.Rect_Orthogonal;
import Graphic.Color;
import std;
import ext.DynamicBuffer;
import ext.Json;

import Core.IO.Specialized;


namespace Colors = Graphic::Colors;

namespace Game::ChamberUtil{
	using ChamberColorRef = Graphic::Color;
	template <typename T>
	[[nodiscard]] ChamberTile<T> parseColor(const ChamberColorRef& color, Geom::Point2 pos){
		return ChamberFactory<T>::genEmptyTile(pos);
	}
}

export namespace Game::ChamberUtil{
	using RefType = unsigned;
	constexpr RefType NoChamberDataIndex = std::numeric_limits<RefType>::max();


	constexpr ChamberColorRef nullRef = Colors::CLEAR;
	constexpr ChamberColorRef invalid = Colors::BLACK;
	constexpr ChamberColorRef placed = Colors::WHITE;

	template <typename ET>
	[[nodiscard]] Graphic::Pixmap saveToPixmap(const ChamberFrame<ET>& frame){
		const Geom::OrthoRectInt bound = frame.getTiledBound();

		Graphic::Pixmap map{bound.getWidth(), bound.getHeight()};

		for (const auto & tile : frame.getData()){
			Geom::Point2 pixmapPos = tile.pos - bound.getSrc();
			map.set(pixmapPos.x, pixmapPos.y, tile.valid() ? placed : invalid);
		}

		return map;
	}

	template <typename ET>
	[[nodiscard]] ChamberFrame<ET> genFrameFromPixmap(const Graphic::Pixmap& map, const Geom::Point2 offset = {}){
		ChamberFrame<ET> frame{};

		map.each([&frame, offset](const Graphic::Pixmap& pixmap, const int x, const int y){
			const auto color = pixmap.get(x, y);
			if(color.equalRelaxed(nullRef))return;

			frame.insert(parseColor<ET>(pixmap.get(x, y), offset + Geom::Point2{x, y}));
		});

		return frame;
	}
}