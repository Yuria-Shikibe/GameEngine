//
// Created by Matrix on 2024/4/14.
//

export module UI.ImageRegion;

export import UI.Widget;
export import UI.RegionDrawable;

import std;
import GL.Texture.Texture2D;
import GL.Texture.TextureRegionRect;

export namespace UI{
	class ImageRegion : public Widget{
		std::unique_ptr<RegionDrawable> drawable{};

	public:
		[[nodiscard]] const std::unique_ptr<RegionDrawable>& getDrawable() const{ return drawable; }

		void setDrawable(std::unique_ptr<RegionDrawable>&& drawable){
			this->drawable = std::move(drawable);
		}

		void drawContent() const override{
			if(drawable){
				drawable->draw(getValidBound().move(absoluteSrc));
			}
		}
	};
}
