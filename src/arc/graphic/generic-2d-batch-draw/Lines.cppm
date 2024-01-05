module;

export module Graphic.Draw.Lines;

export import Graphic.Draw;
export import Geom.Vector2D;

import <vector>;

namespace Graphic::Draw::Line {
	std::vector<Geom::Vec2> pointBuffer{};

	bool buildingLine = false;
	bool closedLine = false;

}

export namespace Graphic::Draw::Line {
	void beginLineVert(const bool closed = true){
		buildingLine = true;
		closedLine = closed;
	}

	void push(Geom::Vec2 vec2) {
		pointBuffer.push_back(std::move(vec2));
	}

	void endLineVert() {
		buildingLine  = false;
		if(pointBuffer.empty())return;

		if(closedLine) {
			pointBuffer.emplace_back(pointBuffer.front());
		}

		for(int i = 0; i < pointBuffer.size() - 1; ++i) {
			Draw::line(pointBuffer[i], pointBuffer[i + 1]);
		}
	}

	template <typename ...Args>
		requires (std::is_same_v<Geom::Vec2, Args> && ...)
	void outline(Args... args) {
		beginLineVert();
		(push(args), ...);
		endLineVert();
	}
}