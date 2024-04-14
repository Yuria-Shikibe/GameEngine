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

export namespace UI{
	//TODO uses another class to do things like this for files:
	//THis cannot support such a hugh range search
	class FileTreeSelector : public Table{
	public:
		std::stack<OS::File> trace{};
		OS::File current{};
		int lastSelectedIndex{-1};

		FileTreeSelector(){
			defaultCellLayout.fillParentX().wrapY().setMargin(5);
			setCellAlignMode(Align::Mode::top_center);
			Table::setEmptyDrawer();
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
				if(record)trace.push(current);
				current = file;
				buildCurrentPage();
				lastSelectedIndex = -1;
			}
		}

		void gotoDirectory(OS::File&& file){
			current = std::move(file);

			buildCurrentPage();
		}

		void buildCurrentPage(){
			clearChildrenSafely();

			// setCellAlignMode(current.exist() ? Align::Mode::top_center : Align::Mode::center);

			if(current.exist()){
				add<Button>([this](Button& button){
					button.setCall([this](const bool isPressed){
						if(!isPressed){
							returnToParentDirectory(current == current.getRoot());
						}
					});

					button.Table::add<Label>([this](Label& label){
						label.setEmptyDrawer();
						label.setWrap(false, true);
						label.setText(std::format("{} {}", ">..", current.filenameFullPure()));
						label.getGlyphLayout()->setSCale(0.5f);
					}).wrapY().setMargin(Align::Spacing{.left = 10.0f});
				}).wrapY().endLine().setAlign(Align::Mode::top_center);
			}

			add<ScrollPane>([this](ScrollPane& pane){
				pane.setEmptyDrawer();
				pane.setItem<Table>([this](Table& table){
					table.setEmptyDrawer();
					table.setFillparentX();
					table.defaultCellLayout.fillParentX().wrapY();

					if(current.exist()){
						auto&& subs = current.subs<true>();
						std::ranges::sort(subs);
						for (auto [index, fi] : subs | std::ranges::views::enumerate){
							buildSingle(table, fi, index);
						}
					}else{
						for(auto& fi : ext::platform::getLogicalDrives()){
							buildSingle(table, fi, -1);
						}
					}
				});
			}).setPad({.top = 5}).setAlign(Align::Mode::top_center).fillParentY();

			layout();
		}

		void buildSingle(Table& table, const OS::File& file, int index){
			table.add<Button>([this, &file, index](Button& inner){
				inner.setCall([this, fileCopy = file, index](const bool isPressed){
					if(!isPressed){
						gotoFile(fileCopy);

						if(lastSelectedIndex == index){
							lastSelectedIndex = -1;
						}else{
							lastSelectedIndex = index;
						}
					}
				});

				inner.setHoverTableBuilder({
					.builder = [file](Table& hint){
						auto pixmap = ext::platform::getThumbnail(file);

						if(pixmap.valid()){
							auto drawable = std::make_unique<UniqueRegionDrawable>(
							std::move(pixmap).genTex());
							drawable->texture->setScale(GL::mipmap_linear_linear, GL::nearest);
							hint.setCellAlignMode(Align::Mode::top_left);
							hint.setEmptyDrawer();
							hint.add<ImageRegion>([drawable = std::move(drawable)](ImageRegion& image) mutable {
								image.setSize(drawable->texture->getWidth(), drawable->texture->getHeight());
								image.setDrawable(std::move(drawable));
								image.selfMaskOpacity = image.color.a = 0.85f;
							}).expand().endLine();
						}else{
							hint.setVisible(false);
							hint.setSize(0);
							hint.setEmptyDrawer();
						}
					},
					.followCursor = true
				});

				if(!file.isDir()){
					inner.setDrawer(UI::Styles::drawer_elem_s1_light_gray.get());
					inner.setActivatedChecker([index, this]{
						return lastSelectedIndex == index;
					});
				}else{

				}

				inner.add<Label>([&file](Label& label){
					label.setEmptyDrawer();
					if(file.isDir()){
						label.setText(std::format("{}$<c#AQUA_SKY>$<sub><DIR>", file.filenameFullPure()));
					}else{
						if(file.hasExtension()){
							label.setText(std::format("$<c#LIGHT_GRAY>{}$<c#PALE_GREEN>$<sub>{}", file.stem(), file.extension()));
						}else{
							label.setText(std::format("$<c#LIGHT_GRAY>{}$<c#PALE_GREEN>", file.stem()));
						}
					}
					label.setWrap(false, true);
					label.getGlyphLayout()->setSCale(0.65f);
					label.setTextAlign(Align::Mode::center_left);
				}).setMargin(Align::Spacing{.left = 10.0f}).wrapY().setAlign(Align::Mode::center_left); //
			}).setHeight(60).endLine().setPad({.bottom = 5.5f,});
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
