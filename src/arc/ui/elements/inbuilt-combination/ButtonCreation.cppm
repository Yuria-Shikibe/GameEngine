//
// Created by Matrix on 2024/6/1.
//

export module UI.ButtonCreation;

export import UI.Button;
export import UI.Cell;
export import UI.ImageRegion;

import std;

export namespace UI::Create{

	template <Concepts::Derived<RegionDrawable> Drawable, Concepts::Invokable<void(Button&, bool)> Func, Concepts::InvokeNullable<void(Button&)> Initer = std::nullptr_t>
	std::pair<std::unique_ptr<UI::Button>, UI::ImageRegion&> imageButton(const Drawable& drawable, Func&& func, Initer&& initer = nullptr){
		auto button = std::make_unique<UI::Button>();

		button->setCall(std::forward<Func>(func));

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
}
