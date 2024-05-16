//
// Created by Matrix on 2024/5/14.
//

export module UI.ControlBindTable;

import UI.Table;
import UI.Button;
import UI.ScrollPane;
import UI.Label;
import UI.Format;
import UI.Palette;
import OS.InputListener;

import OS.Ctrl.Operation;
import OS.Ctrl;
import std;

export namespace UI{
	class ControlBindTable : public Table, public OS::InputListener{

		void buildSignle(UI::Table& toBuild, Ctrl::Operation& operation){
			toBuild.add<UI::Table>([&](UI::Table& table){
				table.setEmptyDrawer();
				table.setLayoutByRelative(false);
				table.add<UI::Label>([&](UI::Label& label){
					label.setEmptyDrawer();
					label.setText(operation.instruction.name);
					label.setWrap();
					label.setTextScl(0.75f);
					label.getBorder().left = 15;
				}).setAlign(Align::Mode::left).setSizeScale(0.4f, 1.0f);

				table.add<UI::Button>([&operation, this](UI::Button& button){
					button.setCall([&operation, this](UI::Button& b, bool){
						beginBind(&b, &operation);
						ignoreNext = true;
					});
					buildBindInfo(button, operation);
					button.appendUpdator = [this, &button, &operation](){
						if(button.isCursorInbound()){
							if(button.keyDown(Ctrl::Key::Backspace, Ctrl::Act::Release, Ctrl::Mode::Ignore)){
								beginBind(&button, &operation);
								operation.setDef();
								endBind();
							}
						}
					};
				}).setAlign(Align::Mode::right).setSizeScale(0.45f, 1.0f);

				table.add<UI::Label>([&operation, this](UI::Label& label){
					label.setText(label.getBundleEntry(Ctrl::ActNames[operation.defaultBind.state()]));
					label.setTextAlign(Align::Mode::center);
					label.setTextScl(0.65f);
				}).setAlign(Align::Mode::right).setMargin({.right = 10.0f}).setSizeScale(0.15f, 1.0f).setSrcScale(0.45f, 0.0f);

				if(operation.instruction.hasDesc()){
					table.setTooltipBuilder({
						.followTarget = UI::TooltipFollowTarget::cursor,
						.builder = [&](UI::Table& hint){
							hint.add<UI::Label>([&](UI::Label& label){
								label.setWrap();
								label.setText(operation.instruction.desc);
								label.getGlyphLayout()->setSCale(0.65f);
								label.setEmptyDrawer();
								label.getBorder().expand(4.0f);
							}).expand().endLine();
						}
					});
				}

			}).setHeight(75.0f).setPad({.bottom = 3, .top = 3}).endLine();
		}

		void beginBind(UI::Button* button, Ctrl::Operation* operation);

		void endBind();

		bool ignoreNext = false;

		Ctrl::Operation* currentToBind{nullptr};
		Ctrl::OperationGroup tempOperations{};
		Ctrl::OperationGroup* srcGroup{};
		Button* currentButton{nullptr};

		static void buildBindInfo(Button& button, Ctrl::Operation& operation){
			button.add<UI::Label>([&operation](UI::Label& label){
				auto modes = Ctrl::getModesStr(operation.customeBind.mode());
				auto key = UI::Fmt::keyType(Ctrl::KeyNames[operation.customeBind.getKey()]);
				auto full = modes | std::ranges::views::join | std::ranges::to<std::string>();

				if(modes.empty() || operation.customeBind.isIgnoreMode()){
					label.setText(std::format("{}", key));
				}else{
					label.setText(std::format("[{}] + {}", std::move(full), key));
				}
				label.setTextAlign(Align::Mode::center);
				label.getGlyphLayout()->setSCale(0.5f);
				label.setEmptyDrawer();
				label.setFillparent();
				label.setWrap(true, false);
			}).fillParent();

			//TODO impl conflict check
			// if(operation.group->isConfilcted(operation.customeBind.getFullKey())){
			// 	button.color = UI::Pal::RED_DUSK;
			// }else
			if(operation.hasCustomData()){
				button.color = UI::Pal::PALE_GREEN;
			}else{
				button.color = UI::Pal::WHITE;
			}
		}
	public:
		~ControlBindTable() override{endBind();}

		void applyBind(){
			tempOperations.applyToTarget();
			srcGroup->operator=(tempOperations);
		}

		void build(Ctrl::OperationGroup& group){
			tempOperations = group;
			srcGroup = &group;

			add<UI::ScrollPane>([this](UI::ScrollPane& pane){
				pane.setEmptyDrawer();
				pane.setItem<UI::Table>([this](UI::Table& menu){
					menu.setEmptyDrawer();

					auto range = tempOperations.getShownBinds()
						 | std::ranges::views::transform([](Ctrl::Operation& t){ return std::addressof(t); })
						 | std::ranges::to<std::vector<Ctrl::Operation*>>();

					std::ranges::sort(range, std::less{}, &Ctrl::Operation::getID);
					for (const auto& bind : range){
						buildSignle(menu, *bind);
					}
				});
			}).fillParentY().setAlign(Align::Mode::top_center).endLine();

			add<UI::Table>([this](UI::Table& table){
				table.setEmptyDrawer();

				table.add<UI::Button>([this](UI::Button& pane){
					pane.setCall([this](auto&, bool){
						applyBind();
					});

					pane.add<UI::Label>([](UI::Label& label){
						label.setTextAlign(Align::Mode::center);
						label.setText(label.getBundleEntry("apply"));
						label.getGlyphLayout()->setSCale(0.65f);
						label.setEmptyDrawer();
					}).fillParent();
				}).setSizeScale(0.5f, 1.0f).setAlign(Align::Mode::center);
			}).setHeight(60.0f).setAlign(Align::Mode::top_center);
		}

		[[nodiscard]] bool bindChanged() const{
			return !srcGroup->isEquivalentTo(tempOperations);
		}

		[[nodiscard]] bool isBinding() const{
			return currentToBind != nullptr;
		}

		void inform(const int key, const int action, const int mods) override{
			if(key == Ctrl::Key::Esc){
				endBind();
				ignoreNext = false;
			}

			if(action == Ctrl::Act::Release){
				if(ignoreNext){
					ignoreNext = false;
					return;
				}

				currentToBind->setCustom(key, mods);
				endBind();
				ignoreNext = false;
			}
		}

		bool onEsc() override{
			if(isBinding())return false;
			return Table::onEsc();
		}
	};
}
