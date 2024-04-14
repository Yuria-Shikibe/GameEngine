//
// Created by Matrix on 2024/4/14.
//

export module UI.RegionDrawable;

export import Geom.Rect_Orthogonal;
export import GL.Texture.TextureRegion;
export import GL.Texture.TextureNineRegion;
export import GL.Texture.TextureRegionRect;
export import GL.Texture.Texture2D;

import ext.RuntimeException;
import std;

export namespace UI{
	struct RegionDrawable {
		virtual ~RegionDrawable() = default;

		virtual void draw(Geom::OrthoRectFloat rect) const{

		}
	};

	struct UniqueRegionDrawable : RegionDrawable{
		std::unique_ptr<GL::Texture2D> texture{};
		GL::TextureRegionRect wrapper{};

		template <typename ...T>
		explicit UniqueRegionDrawable(T&& ...args) : texture{std::make_unique<GL::Texture2D>(std::forward<T>(args)...)}, wrapper{texture.get()}{}

		explicit UniqueRegionDrawable(std::unique_ptr<GL::Texture2D>&& texture) : texture{std::move(texture)}, wrapper{texture.get()}{}

		explicit UniqueRegionDrawable(GL::Texture2D&& texture) : texture{std::make_unique<GL::Texture2D>(std::move(texture))}, wrapper{this->texture.get()}{}

		void draw(const Geom::OrthoRectFloat rect) const override;
	};

	struct TextureNineRegionDrawable : RegionDrawable{
		GL::TextureNineRegion* texRegion{nullptr};

		[[nodiscard]] explicit TextureNineRegionDrawable(GL::TextureNineRegion* const rect)
			: texRegion(rect) {
		}

		void draw(const Geom::OrthoRectFloat rect) const override{
#ifdef _DEBUG
			if(!texRegion)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
			texRegion->render_RelativeExter(rect);
		}
	};
}
