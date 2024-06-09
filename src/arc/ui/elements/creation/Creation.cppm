//
// Created by Matrix on 2024/6/1.
//

export module UI.Creation;

export import UI.Button;
export import UI.Cell;
export import UI.Icons;
export import UI.Palette;
export import UI.ImageRegion;

import Graphic.Color;
import std;
import ext.MetaProgramming;

export namespace UI::Create{

	template <auto mptr>
		requires std::equality_comparable<typename ext::GetMemberPtrInfo<decltype(mptr)>::ValueType>
	struct ValueChecker{
		using V = typename ext::GetMemberPtrInfo<decltype(mptr)>::ValueType;
		using T = typename ext::GetMemberPtrInfo<decltype(mptr)>::ClassType;

		const T& src;
		V val{};

		[[nodiscard]] ValueChecker(const T& src, const V& val) noexcept
			: src{src},
			  val{val}{}

		[[nodiscard]] ValueChecker(const T* src, const V& val) noexcept
			: ValueChecker{*src, val}{}

		bool operator()(const Elem&) const noexcept{
			return (std::invoke(mptr, src) == val);
		}
	};

	struct CheckBox : UI::ElemCreater<UI::Button>{
		UI::Icon& icon;
	};

	struct LineCreater : UI::ElemCreater<UI::ImageRegion>{
		Graphic::Color defColor{UI::Pal::THEME};
		bool clearMargin = true;
		void operator()(ImageRegion& image) const;
		void operator()(LayoutCell& cell) const;
	};

	template <Concepts::Derived<RegionDrawable> Drawable, typename Func, Concepts::InvokeNullable<void(Button&)> Initer = std::nullptr_t>
	std::pair<std::unique_ptr<UI::Button>, UI::ImageRegion&> imageButton(ElemDrawer* drawer, const Drawable& drawable, Func&& func, Initer&& initer = nullptr){
		auto button = std::make_unique<UI::Button>();

		if(drawer)button->setDrawer(drawer);

		if constexpr (requires (Func f){
			button->setCall<bool{}>(f);
		}){
			button->setCall<true>(std::forward<Func>(func));
		}else{
			button->setCall(std::forward<Func>(func));
		}

		UI::LayoutCell& cell = button->add<UI::ImageRegion>([&drawable](UI::ImageRegion& imageRegion){
			imageRegion.setDrawable(drawable);
			imageRegion.setEmptyDrawer();
		});

		cell.fillParent();

		if constexpr (!std::same_as<std::nullptr_t, Initer>){
			initer(*button);
		}

		return {std::move(button), cell.as<UI::ImageRegion>()};
	}


	template <
		Concepts::Derived<RegionDrawable> Drawable,
		typename Func,
		Concepts::InvokeNullable<void(Button&)> Initer = std::nullptr_t>
	std::pair<std::unique_ptr<UI::Button>, UI::ImageRegion&> imageButton(const Drawable& drawable, Func&& func, Initer&& initer = nullptr){
		return Create::imageButton<Drawable, Func, Initer>(nullptr, drawable, std::forward<Func>(func), std::forward<Initer>(initer));
	}
}
