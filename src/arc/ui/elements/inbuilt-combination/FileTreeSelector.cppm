module;

export module UI.FileTreeSelector;

export import UI.Table;
export import UI.Button;
export import UI.ScrollPane;
export import UI.ImageRegion;
export import UI.Label;
export import OS.File;
import std;
import ext.platform;
import UI.Styles;
import Graphic.Color;
import ext.Heterogeneous;

import UI.Icons;
import UI.ButtonCreation;

export namespace UI{
	//TODO uses another class to do things like this for files:
	//THis cannot support such a hugh range search
	class FileTreeSelector : public Table{
		void buildBasic(){
			sideMenu = &add<UI::Table>([this](UI::Table& sideMenu){
				sideMenu.setFillparent();
				sideMenu.setCellAlignMode(Align::Layout::top_center);
				sideMenu.defaultCellLayout.setPad({.bottom = 6.f}).setSize(90.f);
				auto [b, a] = UI::Create::imageButton(UI::Icons::new_afferent, [this](auto&, bool){

				});
				b->setDrawer(UI::Styles::drawer_elem_s1_noEdge.get());

				sideMenu.transferElem(std::move(b));
			})
			.setWidth(120.f)
			.fillParentY()
			.as<UI::Table>();

			selectMenu = &add<UI::Table>([](UI::Table& section){
				section.setEmptyDrawer();
			}).fillParent().as<UI::Table>();
		}

	public:
		std::stack<OS::File> trace{};

		ext::StringSet<> suffixFilter{};
		OS::File current{};

		std::unordered_set<OS::File> selected{};

		Table* sideMenu{};
		Table* selectMenu{};

		FileTreeSelector(){
			defaultCellLayout.fillParentX().wrapY().setMargin(5);
			setCellAlignMode(Align::Layout::top_center);
			Table::setEmptyDrawer();

			buildBasic();
		}

		void clearSelected(){
			selected.clear();
		}

		void returnToParentDirectory(const bool toRoot = false){
			if(toRoot){
				gotoFile(OS::File{});
			} else{
				gotoFile(current.getParent());
			}
		}

		void gotoFile(const OS::File& file, const bool record = true){
			if(file.isDir() || file == OS::File{}){
				if(record) trace.push(current);
				current = file;
				buildCurrentPage();
				clearSelected();
			}
		}

		void gotoDirectory(OS::File&& file){
			current = std::move(file);

			buildCurrentPage();
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

					button.Table::add<Label>([this](Label& label){
						label.setEmptyDrawer();
						label.setWrap(false, true);
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
							buildSingle(table, fi, index);
						}
					} else{
						for(auto& fi : ext::platform::getLogicalDrives()){
							buildSingle(table, fi, -1);
						}
					}
				});
			}).setPad({.top = 5 * static_cast<float>(current.exist())}).setAlign(Align::Layout::top_center).fillParentY();

			selectMenu->changed(ChangeSignal::notifySelf);
		}

		void buildSingle(Table& table, const OS::File& file, int index){
			table.add<Button>([this, &file, index](Button& inner){
				inner.setDrawer(UI::Styles::drawer_elem_s1_noEdge.get());
				inner.setCall([this, fileCopy = file, index](auto&, auto){
					gotoFile(fileCopy);

					if(const auto itr = selected.find(fileCopy); itr != selected.end()){
						selected.erase(itr);
					}else{
						selected.insert(itr, fileCopy);
					}

				});

				if(auto pixmap = ext::platform::getThumbnail(file); pixmap.valid())
					inner.setTooltipBuilder({
							.followTarget = TooltipBuilder::FollowTarget::cursor,
							.builder = [pixmap = std::move(pixmap)](Table& hint){
								if(pixmap.valid()){
									auto drawable = std::make_unique<UniqueRegionDrawable>(
										std::move(pixmap).genTex());
									drawable->texture->setFilter(GL::mipmap_linear_linear, GL::nearest);
									hint.setCellAlignMode(Align::Layout::top_left);
									hint.setEmptyDrawer();
									hint.add<ImageRegion>([drawable = std::move(drawable)](ImageRegion& image) mutable{
										image.setSize(drawable->texture->getWidth(), drawable->texture->getHeight());
										image.setDrawable(std::move(drawable));
										image.selfMaskOpacity = image.color.a = 0.85f;
									}).expand().endLine();
								} else{
									hint.setVisible(false);
									hint.setSize(0);
									hint.setEmptyDrawer();
								}
							},
						});

				if(!file.isDir()){
					inner.setActivatedChecker([index, fileCopy = file, this]{
						return selected.contains(fileCopy);
					});
				} else{}

				inner.emplace<ImageRegion>(file.isDir() ? UI::Icons::folder_close : UI::Icons::file_question)
				     .setMargin(2.f)
				     .setSize(48.f);

				inner.add<Label>([&file](Label& label){
					label.setEmptyDrawer();
					if(file.isDir()){
						label.setText(std::format("{}$<c#AQUA_SKY>", file.filenameFullPure()));
					} else{
						label.setText(std::format("$<c#LIGHT_GRAY>{}$<c#PALE_GREEN>$<sub>{}", file.stem(), file.extension()));
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
				gotoFile(trace.top(), false);
				trace.pop();
				return false;
			}

			return true;
		}
	};
}
