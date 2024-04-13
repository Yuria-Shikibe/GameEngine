//
// Created by Matrix on 2024/4/13.
//

export module UI.SeperateDrawable;

export namespace UI{
	struct SeperateDrawable{
		virtual ~SeperateDrawable() = default;

		virtual void draw() const = 0;

		virtual void drawBase() const = 0;
	};
}
