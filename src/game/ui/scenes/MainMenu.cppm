//
// Created by Matrix on 2024/5/13.
//

export module Game.Scene.MainMenu;

export import UI.Scene;

import UI.ScrollPane;
import UI.Label;
import std;

export namespace Game::Scenes{
	class MainMenu : public ::UI::Scene{
	public:
		[[nodiscard]] MainMenu(){
			MainMenu::build();
		}

		void build() override{
			setRelativeLayoutFormat(false);
			setBorderZero();
			setEmptyDrawer();

			add<UI::Table>([](UI::Table& label){
					label.add<UI::ScrollPane>([](UI::ScrollPane& pane){
						pane.setItem<UI::Label>([](UI::Label& label){
							label.setText("test");

							label.setEmptyDrawer();
							label.setFillparentX();
							label.usingGlyphHeight = true;
						});
						pane.setEmptyDrawer();
					});
				})
				.setAlign(Align::left).setSizeScale(0.2f, 1.0f)
				.setMargin(0, 10, 0, 10).setSrcScale(0.075f, 0.0f);

			add<UI::Table>([](UI::Table& table){
				table.setEmptyDrawer();

				table.add<UI::Label>([](UI::Label& label){
					label.setText("2024-5-13");
					label.getGlyphLayout()->setSCale(0.3f);
					label.setEmptyDrawer();
					label.setFillparentX();
					label.setTextAlign(Align::Mode::bottom_right);
					label.usingGlyphHeight = true;
				}).wrapY().setAlign(Align::Mode::bottom_right).endLine();

				table.add<UI::Label>([](UI::Label& label){
					label.setText("Cpp Course Design");
					label.getGlyphLayout()->setSCale(0.4f);
					label.setEmptyDrawer();
					label.setFillparentX();
					label.setTextAlign(Align::Mode::bottom_right);
					label.usingGlyphHeight = true;
				}).wrapY().setAlign(Align::Mode::bottom_right).endLine();

				table.add<UI::Label>([](UI::Label& label){
					label.setText("$<c#LIGHT_GRAY>NUAA $<c#AQUA>162320129");
					label.getGlyphLayout()->setSCale(0.4f);
					label.setEmptyDrawer();
					label.setFillparentX();
					label.setTextAlign(Align::Mode::bottom_right);
					label.usingGlyphHeight = true;
				}).wrapY().setAlign(Align::Mode::bottom_right).endLine();

			})
			   .setAlign(Align::Mode::bottom_right)
			   .setSizeScale(0.3f, 0.15f)
			   .setMargin(0, 12, 0, 12);
		}
	};
}
