module;

export module UI.FileTreeSelector;

export import UI.Table;
export import UI.Button;
export import UI.ScrollPane;
export import UI.ImageRegion;
export import UI.Label;
export import UI.Dialog;
import UI.InputArea;

import Font.GlyphArrangement;

export import OS.File;
import std;
import ext.platform;
import UI.Styles;
import Graphic.Color;
import ext.Heterogeneous;

import UI.Icons;
import UI.ButtonCreation;
import UI.Palette;

export namespace UI{
	//TODO uses another class to do things like this for files:
	//THis cannot support such a hugh range search
	class FileTreeSelector : public Table{
	private:
		std::shared_ptr<Font::GlyphLayout> glyphLayout{};
		std::stack<OS::File> trace{};

		Table* sideMenu{};
		Table* selectMenu{};

		void buildBasic(){
			sideMenu = &add<Table>([this](Table& sideMenu){
				sideMenu.setFillparent();
				sideMenu.setCellAlignMode(Align::Layout::top_center);
				sideMenu.defaultCellLayout.setPad({.bottom = 4.f, .top = 4.f}).setSize(80.f);

				sideMenu.transferElem(Create::imageButton(Icons::search,
					ButtonFunc::buildOrDropTooltip, [this](Button& b){
						b.setTooltipBuilder({
							.followTarget = TooltipBuilder::FollowTarget::parent,
							.minHoverTime = UI::DisableAutoTooltip,
							.autoRelease = false,
							.followTargetAlign = Align::Layout::top_right,
							.tooltipSrcAlign = Align::Layout::top_left,
							.builder = [this](Table& inputTable){
								inputTable.setFillparent(false, false);
								inputTable.add<InputArea>([this](InputArea& inputArea){
									if(glyphLayout){
										inputArea.setGlyphLayout(glyphLayout);
									}else{
										glyphLayout = inputArea.getGlyphLayout();
									}

									inputArea.ignoredText = {U'\n'};
									inputArea.setEmptyDrawer();
									inputArea.setWrap();
									inputArea.setTextAlign(Align::Layout::left);
									inputArea.setMaxTextLength(32);
									inputArea.setTextScl(0.75f);
									inputArea.setMinimumSize({10, 10});
									inputArea.setHintText(inputArea.getBundleEntry("\2search"));
								}).wrap();
							}
						});

						ButtonFunc::addButtonTooltipCheck(b);

						b.setDrawer(Styles::drawer_elem_s1_noEdge.get());
					}
				).first).endLine();

				sideMenu.transferElem(Create::imageButton(Icons::sort_three,
					[this](auto&, bool){

					}, std::bind_back(&Button::setDrawer, Styles::drawer_elem_s1_noEdge.get())
				).first).endLine();
			})
			.setWidth(120.f)
			.fillParentY()
			.as<Table>();

			selectMenu = &add<Table>([](Table& section){
				section.setEmptyDrawer();
			}).fillParent().as<Table>();
		}

	public:
		bool singleSelect = true;
		std::unordered_set<OS::File> selected{};
		ext::StringSet<> suffixFilter{};

		OS::File current{};

		void createDialogQuitButton(UI::Dialog& dialog){
			sideMenu->transferElem(Create::imageButton(Icons::new_afferent,
				[this, dialog = &dialog](auto&, bool){
					dialog->destroy();
				}, std::bind_back(&Button::setDrawer, Styles::drawer_elem_s1_noEdge.get())
			).first).endLine();
		}

		FileTreeSelector(){
			defaultCellLayout.fillParentX().wrapY().setMargin(5);
			setCellAlignMode(Align::Layout::top_center);
			Table::setEmptyDrawer();

			buildBasic();
		}

		void clearSelected(){
			selected.clear();
		}

		void addSelected(const OS::File& file){
			if(singleSelect){
				clearSelected();
				selected.insert(file);
			}else if(const auto itr = selected.find(file); itr != selected.end()){
				selected.erase(itr);
			}else{
				selected.insert(itr, file);
			}
		}

		void returnToParentDirectory(const bool toRoot = false){
			if(toRoot){
				gotoDirectory(OS::File{});
			} else{
				gotoDirectory(current.getParent());
			}
		}

		void gotoDirectory(const OS::File& file, const bool record = true){
			if(file.isDir() || file == OS::File{}){
				if(record) trace.push(current);
				current = file;
				buildCurrentPage();
				clearSelected();
			}
		}

		void buildCurrentPage(){
			// return;
			if(!selectMenu) return;
			selectMenu->clearChildrenSafely();

			if(current.exist()){
				selectMenu->add<Button>([this](Button& button){
					button.setCall([this](auto&, auto){
						returnToParentDirectory(current == current.getRoot());
					});

					button.add<Label>([this](Label& label){
						label.setEmptyDrawer();
						label.setFillparentX();
						label.setWrap(true, true);
						label.setText(std::format("{} {}", ">..", current.filenameFullPure()));
						label.getGlyphLayout()->setSCale(0.5f);
					}).wrapY().setMargin(Align::Spacing{.left = 10.0f});
				}).wrapY().endLine().setAlign(Align::Layout::top_center);
			}

			selectMenu->add<ScrollPane>([this](ScrollPane& pane){
				pane.setEmptyDrawer();
				pane.setItem<Table>([this](Table& table){
					table.setEmptyDrawer();
					table.setFillparentX();
					table.defaultCellLayout.fillParentX().wrapY();

					if(current.exist()){
						auto&& subs = current.subs([this](const OS::File& file){
							if(file.isDir() || suffixFilter.empty()) return true;

							const auto ext = file.extension();
							if(ext.empty()) return false;
							return suffixFilter.contains(ext);
						});

						std::ranges::sort(subs);
						for(auto [index, fi] : subs | std::ranges::views::enumerate){
							buildSingle(table, fi);
						}
					} else{
						for(auto& fi : ext::platform::getLogicalDrives()){
							buildSingle(table, fi);
						}
					}
				});
			}).setPad({.top = 5 * static_cast<float>(current.exist())}).setAlign(Align::Layout::top_center).fillParentY();

			selectMenu->changed(ChangeSignal::notifySelf);
		}

		void buildSingle(Table& menu, const OS::File& file){
			menu.add<Button>([this, &file, &menu](Button& fButton){
				fButton.setDrawer(Styles::drawer_elem_s1_noEdge.get());

				fButton.visibilityChecker = [this, fileCopy = file, &menu, &fButton]{
					auto result = !glyphLayout || glyphLayout->ignore();
					if(!result){
						result = fileCopy.filename().contains(glyphLayout->getView(true));
					}

					if(result != fButton.isVisiable()){
						menu.notifyLayoutChanged();
					}

					return result;
				};

				fButton.setCall([this, fileCopy = file](auto&, auto){
					gotoDirectory(fileCopy);

					addSelected(fileCopy);
				});

				fButton.setTooltipBuilder({
						.followTarget = TooltipBuilder::FollowTarget::cursor,
						.builder = [fileCopy = file](Table& hint){
							if(auto pixmap = ext::platform::getThumbnail(fileCopy); pixmap.valid()){
								auto drawable = std::make_unique<UniqueRegionDrawable>(GL::Texture2D{
										pixmap.getWidth(), pixmap.getHeight(), std::move(pixmap).data()
									});

								drawable->texture->setFilter(GL::mipmap_linear_linear, GL::nearest);

								hint.add<ImageRegion>([drawable = std::move(drawable)](ImageRegion& image) mutable{
									image.setSize(drawable->texture->getWidth(), drawable->texture->getHeight());
									image.setDrawable(std::move(drawable));
									image.setEmptyDrawer();
									image.selfMaskOpacity = image.color.a = 0.85f;
								}).setSize(160.f, 160.f, true).endLine();
							}else{
								hint.setVisible(false);
							}
						},
					});

				if(!file.isDir()){
					fButton.setActivatedChecker([fileCopy = file, this]{
						return selected.contains(fileCopy);
					});
				}

				auto extension = file.extension();

				fButton.add<ImageRegion>([&file, extension = std::string_view(extension)](ImageRegion& imageRegion){
					imageRegion.setDrawable(file.isDir() ? Icons::folder_close : Icons::getIconByFileSuffix(extension));
					imageRegion.color = (file.isDir() ? Pal::AQUA_SKY : Pal::LIGHT_GRAY).createLerp(Pal::WHITE, 0.25f);
					imageRegion.setEmptyDrawer();
				}).setMargin(2.f)
				     .setSize(48.f);

				fButton.add<Label>([&file, extension = std::move(extension)](Label& label){
					label.setEmptyDrawer();
					if(file.isDir()){
						label.setText(std::format("{}$<c#AQUA_SKY>", file.filenameFullPure()));
					} else{
						label.setText(std::format("$<c#LIGHT_GRAY>{}$<c#PALE_GREEN>$<sub>{}", file.stem(), extension));
					}
					label.setWrap(false, true);
					label.getGlyphLayout()->setSCale(0.65f);
					label.setTextAlign(Align::Layout::center_left);
				}).setPad({.left = 10.0f}).wrapY();
			}).setHeight(64).endLine().setPad({.bottom = 3.f, .top = 3.f,});
		}

		void update(const Core::Tick delta) override{
			Table::update(delta);

			// if(section)section->layout();
		}

		bool onEsc() override{
			if(!trace.empty()){
				gotoDirectory(trace.top(), false);
				trace.pop();
				return false;
			}

			return true;
		}
	};
}
