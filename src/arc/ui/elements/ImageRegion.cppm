//
// Created by Matrix on 2024/4/14.
//

export module UI.ImageRegion;

export import UI.Elem;
export import UI.RegionDrawable;

import std;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegion;

export namespace UI{
	class ImageRegion : public Elem{
		std::unique_ptr<RegionDrawable> drawable{};

	public:
		Align::Scale scaling{Align::Scale::fit};
		Align::Layout imageAlign{Align::Layout::center};

		[[nodiscard]] const std::unique_ptr<RegionDrawable>& getDrawable() const{ return drawable; }

		void setDrawable(std::unique_ptr<RegionDrawable>&& drawable){
			this->drawable = std::move(drawable);
		}

		template <Concepts::Derived<RegionDrawable> Drawable>
		void setDrawable(const Drawable& drawable){
			this->drawable = std::make_unique<Drawable>(drawable);
		}

		void drawContent() const override{
			if(drawable){
				const auto size = Align::embedTo(scaling, drawable->getDefSize(), getValidSize());
				const auto offset = Align::getOffsetOf(imageAlign, size, getValidBound());

				drawable->draw(Geom::OrthoRectFloat{size.x, size.y}.setSrc(offset + absoluteSrc));
			}
		}
	};
}
