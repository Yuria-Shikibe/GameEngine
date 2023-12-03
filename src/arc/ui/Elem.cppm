//
// Created by Matrix on 2023/12/3.
//

module;

export module UI.Elem;

import Geom.Shape.Rect_Orthogonal;
#include <algorithm>
#include <execution>
import <vector>;

using Rect = Geom::Shape::OrthoRectFloat;

export namespace UI {
	class Elem {
	protected:
		Rect bound{};

		std::vector<Elem*> children{};




		virtual void update(const float delta) {
			std::for_each(std::execution::par_unseq, children.begin(), children.end(), [delta](Elem* elem) {
				elem->update(delta);
			});
		}
	};
}
