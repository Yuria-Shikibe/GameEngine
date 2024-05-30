//
// Created by Matrix on 2024/5/15.
//

export module UI.Dialog.CtrlBind;

export import UI.Dialog;
import UI.Button;
import UI.Label;
import UI.ScrollPane;
import UI.ControlBindTable;

import Assets.Ctrl;

export namespace UI{
	class CtrlBindDialog : public Dialog{
		UI::ControlBindTable* currentBindTable{nullptr};
		UI::Table* bindMenu{nullptr};
		std::string currentSelectedName{};

	public:
		[[nodiscard]] explicit CtrlBindDialog(const bool fillScreen)
			: Dialog{fillScreen}{}

		[[nodiscard]] CtrlBindDialog() = default;

		[[nodiscard]] CtrlBindDialog(const bool fillScreen, UI::Root* root)
			: Dialog{fillScreen, root}{}

		void build() override{
			content.setEmptyDrawer();
			content.setLayoutByRelative(false);

			content.add<UI::ScrollPane>([this](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([this](Table& table){
					table.setEmptyDrawer();
					for(const auto& group : Assets::Ctrl::allGroups | std::views::values){
						table.add<UI::Button>([&, this](UI::Button& button){
							button.add<UI::Label>([&](UI::Label& label){
								label.setText(group->instruction.name);
								label.setEmptyDrawer();
								label.setTextAlign(Align::Layout::center);
							});
							button.setCall([&, this](UI::Button&, bool){
								currentSelectedName = group->getName();
								bindMenu->clearChildrenInstantly();
								currentBindTable = &bindMenu->add<UI::ControlBindTable>([&](UI::ControlBindTable& t){
									t.build(*group);
									t.setEmptyDrawer();
								}).as<UI::ControlBindTable>();
							});
							button.setActivatedChecker([&, this]{
								return group->getName() == currentSelectedName;
							});
						}).setHeight(80.0f).setPad({.bottom = 10.0f}).endLine();
					}
				});

			}).setSizeScale(0.2f, 0.9f).setMargin(4.0f).setAlign(Align::Layout::top_left);

			content.add<UI::Button>([this](UI::Button& button){
				button.add<UI::Label>([](UI::Label& label){
					label.setText(label.getBundleEntry("exit", true));
					label.setEmptyDrawer();
					label.setTextAlign(Align::Layout::center);
				});
				button.setCall([this](UI::Button& b, bool){
					if(tryEsc())destroy();
				});
			}).setSizeScale(0.2f, 0.1f).setMargin(4.0f).setAlign(Align::Layout::bottom_left);

			bindMenu = &content.add<UI::Table>([](UI::Table& table){
				table.add<UI::Label>([](UI::Label& label){
					label.setText(label.getBundleEntry("unavailable", true));
					label.setEmptyDrawer();
					label.setTextAlign(Align::Layout::center);
				});
			}).setSizeScale(0.8f, 1.0f).setMargin(4.0f).setAlign(Align::Layout::right).as<UI::Table>();
		}

		bool tryEsc() override{
			if(currentBindTable){
				if(currentBindTable->bindChanged()){
					UI::Root* root = content.getRoot();
					root->showDialog(true, [this](UI::Dialog& dialog){
						dialog.content.setEmptyDrawer();
						dialog.content.setLayoutByRelative(false);
						dialog.content.add<UI::Table>([this, &dialog](UI::Table& inner){
							inner.setLayoutByRelative(false);
							inner.add<UI::Label>([](UI::Label& label){
								label.setText(label.getBundleEntry("bind-no-confirm-warn", true));
								label.setTextScl(0.8f);
								label.setEmptyDrawer();
								label.setTextAlign(Align::Layout::center);
							}).setSizeScale(1.0f, 0.5f).setAlign(Align::Layout::top_center).endLine();

							inner.add<UI::Table>([this, &dialog](UI::Table& bt){
								bt.setEmptyDrawer();
								bt.add<UI::Button>([this](UI::Button& confirm){
									confirm.setFillparentY();
									confirm.add<UI::Label>([](UI::Label& label){
										label.setText(label.getBundleEntry("yes", true));
										label.setTextScl(0.8f);
										label.setEmptyDrawer();
										label.setTextAlign(Align::Layout::center);
									});
									confirm.setCall([this](auto&, auto){
										this->destroy();
									});
								}).setMargin({.right = 3});

								bt.add<UI::Button>([&dialog](UI::Button& confirm){
									confirm.setFillparentY();
									confirm.add<UI::Label>([](UI::Label& label){
										label.setText(label.getBundleEntry("no", true));
										label.setTextScl(0.8f);
										label.setEmptyDrawer();
										label.setTextAlign(Align::Layout::center);
									});
									confirm.setCall([&dialog](auto&, auto){
										dialog.destroy();
									});
								}).setMargin({.left = 3});
							}).setSizeScale(1.0f, 0.4f).setAlign(Align::Layout::bottom_center);
						}).setSizeScale(0.5f, 0.175f).setAlign(Align::Layout::center);
					});

					return false;
				}
			}

			return true;
		}
	};
}
