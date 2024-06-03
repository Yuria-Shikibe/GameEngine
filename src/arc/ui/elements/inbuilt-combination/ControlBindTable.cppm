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

		void buildSignle(Table& toBuild, Ctrl::Operation& operation){
			toBuild.add<Table>([&](Table& table){
				table.setEmptyDrawer();
				table.setLayoutByRelative(false);
				table.add<Label>([&](Label& label){
					label.setEmptyDrawer();
					label.setText(operation.instruction.name);
					label.setWrap();
					label.setTextScl(0.75f);
					label.getBorder().left = 15;
				}).setAlign(Align::Layout::left).setSizeScale(0.4f, 1.0f);

				table.add<Button>([&operation, this](Button& button){
					button.setCall([&operation, this](Button& b){
						beginBind(&b, &operation);
						ignoreNext = true;
					});
					buildBindInfo(button, operation);
					button.appendUpdator = [this, &operation](Elem& button_1){
						if(button_1.isCursorInbound()){
							if(button_1.keyDown(Ctrl::Key::Backspace, Ctrl::Act::Release, Ctrl::Mode::Ignore)){
								beginBind(static_cast<Button*>(&button_1), &operation);
								operation.setDef();
								endBind();
							}
						}
					};
				}).setAlign(Align::Layout::right).setSizeScale(0.45f, 1.0f);

				table.add<Label>([&operation, this](Label& label){
					label.setText(label.getBundleEntry(Ctrl::ActNames[operation.defaultBind.state()]));
					label.setTextAlign(Align::Layout::center);
					label.setTextScl(0.65f);
				}).setAlign(Align::Layout::right).setMargin({.right = 10.0f}).setSizeScale(0.15f, 1.0f).setSrcScale(0.45f, 0.0f);

				if(operation.instruction.hasDesc()){
					table.setTooltipBuilder({
						.followTarget = TooltipFollowTarget::cursor,
						.builder = [&](Table& hint){
							hint.add<Label>([&](Label& label){
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

		void beginBind(Button* button, Ctrl::Operation* operation);

		void endBind();

		bool ignoreNext = false;

		Ctrl::Operation* currentToBind{nullptr};
		Ctrl::OperationGroup tempOperations{};
		Ctrl::OperationGroup* srcGroup{};
		Button* currentButton{nullptr};

		static void buildBindInfo(Button& button, Ctrl::Operation& operation){
			button.add<Label>([&operation](Label& label){
				auto modes = Ctrl::getModesStr(operation.customeBind.mode());
				auto key = Fmt::keyType(Ctrl::KeyNames[operation.customeBind.getKey()]);
				auto full = modes | std::ranges::views::join | std::ranges::to<std::string>();

				if(modes.empty() || operation.customeBind.isIgnoreMode()){
					label.setText(std::format("{}", key));
				}else{
					label.setText(std::format("[{}] + {}", std::move(full), key));
				}
				label.setTextAlign(Align::Layout::center);
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
				button.color = Pal::PALE_GREEN;
			}else{
				button.color = Pal::WHITE;
			}
		}

		void buildCur(){
			clearChildrenInstantly();
			add<ScrollPane>([this](ScrollPane& pane){
				pane.setEmptyDrawer();
				pane.setItem<Table>([this](Table& menu){
					menu.setEmptyDrawer();

					auto range = tempOperations.getShownBinds()
						 | std::ranges::views::transform([](Ctrl::Operation& t){ return std::addressof(t); })
						 | std::ranges::to<std::vector<Ctrl::Operation*>>();

					std::ranges::sort(range, std::less{}, &Ctrl::Operation::getID);
					for (const auto& bind : range){
						buildSignle(menu, *bind);
					}
				});
			}).fillParentY().setAlign(Align::Layout::top_center).endLine();

			add<Table>([this](Table& table){
				table.setEmptyDrawer();

				table.add<Button>([this](Button& pane){
					pane.setCall([this]{
						applyBind();
					});

					pane.add<Label>([](Label& label){
						label.setTextAlign(Align::Layout::center);
						label.setText(label.getBundleEntry("apply"));
						label.getGlyphLayout()->setSCale(0.65f);
						label.setEmptyDrawer();
					}).fillParent();
				}).setSizeScale(0.75f, 1.0f).setPad({.left = 3, .right = 3}).setAlign(Align::Layout::right);

				table.add<Button>([this](Button& pane){
					pane.setCall([this]{
						tempOperations.setDef();
						buildCur();
					});

					pane.add<Label>([](Label& label){
						label.setTextAlign(Align::Layout::center);
						label.setText(label.getBundleEntry("apply-def"));
						label.getGlyphLayout()->setSCale(0.65f);
						label.setEmptyDrawer();
					}).fillParent();
				}).setSizeScale(0.75f, 1.0f).setPad({.left = 3, .right = 3}).setAlign(Align::Layout::left);
			}).setHeight(60.0f).setAlign(Align::Layout::top_center);
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

			buildCur();
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
