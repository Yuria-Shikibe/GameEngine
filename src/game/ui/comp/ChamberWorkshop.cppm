//
// Created by Matrix on 2024/5/23.
//

export module Game.UI.ChamberWorkshop;

export import UI.Table;
export import UI.Screen;
import UI.ScrollPane;
import UI.Button;
import UI.Label;

export import Game.Chamber;
export import Game.Chamber.Frame;

export namespace Game::Scene{
	template <typename T>
	class ChamberWorkshop : public UI::Table{
		Game::ChamberFrameData<T> frame{};

		UI::Screen* screen{};
		UI::Table* selectionTable{};

		void buildElem(){
			setEmptyDrawer();
			UI::Table::add<UI::ScrollPane>([this](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([this](Table& table){
					// table.setEmptyDrawer();
					
				});
			}).setSizeScale(0.2f, 0.9f).setMargin(4.0f).setAlign(Align::Mode::top_left);

			UI::Table::add<UI::Button>([this](UI::Button& button){
				// button.add<UI::Label>([](UI::Label& label){
				// 	label.setText(label.getBundleEntry("exit", true));
				// 	label.setEmptyDrawer();
				// 	label.setTextAlign(Align::center);
				// });
				// button.setCall([this](UI::Button& b, bool){
				// 	if(tryEsc())destroy();
				// });
			}).setSizeScale(0.2f, 0.1f).setMargin(4.0f).setAlign(Align::Mode::bottom_left);

			screen = &UI::Table::add<UI::Screen>([](UI::Screen& screen){

			}).setSizeScale(0.8f, 1.0f).setMargin(4.0f).setAlign(Align::Mode::right).template as<UI::Screen>();
		}
	public:
		using EntityType = T;

		[[nodiscard]] ChamberWorkshop(){
			setLayoutByRelative(false);
		}

		void build(const Game::ChamberFrameData<T>& data){
			frame = data;

			buildElem();
		}

		// void drawContent() const override{}
	};
}
