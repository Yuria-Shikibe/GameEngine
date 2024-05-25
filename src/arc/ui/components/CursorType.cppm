//
// Created by Matrix on 2024/4/15.
//

export module UI.CursorType;

import Geom.Vector2D;
import std;

export namespace UI{
	enum struct CursorType : unsigned {
		regular,
		clickable,
		regular_tip,
		clickable_tip,

		select,
		select_regular,

		textInput,
		scroll,
		scrollHori,
		scrollVert,
		drag,
		//...
		freeBegin,
	};

	struct CursorAdditionalDrawer {
		virtual ~CursorAdditionalDrawer() = default;
		virtual void operator()(float x, float y, float w, float h) = 0;
	};

	struct CursorDrawabe{
		virtual ~CursorDrawabe() = default;

		std::unique_ptr<CursorAdditionalDrawer> drawer{nullptr};

		virtual void draw(float x, float y, Geom::Vec2 screenSize, float progress, float scl) const = 0;

		void draw(const float x, const float y, const Geom::Vec2 screenSize, const float progress = 0.0f) const{
			draw(x, y, screenSize, progress, 1.0f);
		}
	};


	//TODO is unordered_map better for extension?
	inline std::vector<std::unique_ptr<CursorDrawabe>> allCursors{static_cast<size_t>(CursorType::freeBegin)};

	CursorDrawabe& getCursor(CursorType type){
		return *allCursors[static_cast<size_t>(type)];
	}

	std::unique_ptr<CursorDrawabe>& getCursorRaw(CursorType type){
		return allCursors[static_cast<size_t>(type)];
	}

	void setCursorRaw(CursorType type, std::unique_ptr<CursorDrawabe>&& cursor){
		auto s = static_cast<unsigned>(type);
		if(s >= allCursors.size()){
			allCursors.resize(s + 1);
		}

		allCursors.at(s) = std::move(cursor);
	}
}
