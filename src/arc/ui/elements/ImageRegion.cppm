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
		std::function<decltype(drawable)(ImageRegion&)> drawableGetter{};

	public:
		Align::Scale scaling{Align::Scale::fit};
		Align::Layout imageAlign{Align::Layout::center};

		using Elem::Elem;

		template <Concepts::Derived<RegionDrawable> Drawable>
		[[nodiscard]] explicit ImageRegion(const Drawable& drawable, const bool useEmptyDrawer = true){
			this->setDrawable<Drawable>(drawable);
			if(useEmptyDrawer)Elem::setEmptyDrawer();
		}

		[[nodiscard]] const std::unique_ptr<RegionDrawable>& getDrawable() const{ return drawable; }

		bool isDynamic() const noexcept{
			return drawableGetter != nullptr;
		}

		void setDrawable(std::unique_ptr<RegionDrawable>&& drawable){
			this->drawable = std::move(drawable);
		}

		template <Concepts::Derived<RegionDrawable> Drawable>
		void setDrawable(Drawable&& drawable){ //TODO ?
			this->drawable = std::make_unique<std::decay_t<Drawable>>(std::forward<Drawable>(drawable));
		}

		template <Concepts::Derived<RegionDrawable> Drawable>
		void setDrawable(const Drawable& drawable){
			this->drawable = std::make_unique<Drawable>(drawable);
		}

		void update(const Core::Tick delta) override{
			Elem::update(delta);

			if(isDynamic()){
				drawable = drawableGetter(*this);
			}
		}

		void drawContent() const override;
	};
}
