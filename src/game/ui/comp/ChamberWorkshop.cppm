// ReSharper disable CppDFANullDereference
// ReSharper disable CppDFAUnreachableCode
// ReSharper disable CppDFAUnreachableFunctionCall
export module Game.UI.ChamberWorkshop;

export import UI.Table;
export import UI.Screen;
import UI.ScrollPane;
import UI.Button;
import UI.Label;
import UI.Cell;
import UI.SliderBar;
import UI.ImageRegion;
import UI.Icons;
import UI.Palette;
import UI.Canvas;
import UI.Selection;
import UI.Styles;

export import Game.Chamber;
export import Game.Chamber.Frame;

import Core;
import Graphic.Draw;
import Assets.Graphic;
import GL.Shader.UniformWrapper;

import ext.BooleanOperation;

//TODO move the timer to other place
//TODO continas a timer in root maybe
import OS;

namespace Game::Scene{
	export
	template <typename T>
	struct ChamberWorkshopMenuBuilder{
		std::vector<ChamberFactory<T>*> getFactories() = delete;
	};

	class ChamberWorkshopBase : public UI::Table{
	protected:
		static constexpr auto MaxTileSize = 10;

		static Geom::Point2 getSizeFromProgress(Geom::Vec2 progress){
			return progress.scl(MaxTileSize - 1).round<int>().add(1, 1);
		}

		UI::Screen* screen{};
		UI::Table* selectionTable{};
		UI::SliderBar* slider{};

		enum struct ChamberFilter{
			owner,
			invalid
		};

		UI::BoxSelection<int> boxSelection{};
		ChamberFilter filterType{};
		ext::BooleanOperation booleanOperation{};

		Geom::OrthoRectInt getTileBound() const{
			if(slider){
				auto [w, h] = getSizeFromProgress(slider->getProgress());
				return Geom::OrthoRectInt{w, h};
			} else[[unlikely]] {
				return {};
			}
		}

		void buildElem(){
			setEmptyDrawer();

			add<UI::ScrollPane>([this](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([this](Table& table){
					table.defaultCellLayout.fillParentX();
					table.defaultCellLayout.setPad({.bottom = 6.f});
					table.setEmptyDrawer();

					slider = &table.add<UI::SliderBar>([](UI::SliderBar& bar){
						bar.setSegments(Geom::norBaseVec2<unsigned> * (MaxTileSize - 1));

						bar.setTooltipBuilder({
								.followTarget = UI::TooltipBuilder::FollowTarget::parent,
								.minHoverTime = 0.f,
								.followTargetAlign = Align::Layout::top_right,
								.tooltipSrcAlign = Align::Layout::top_left,
								.builder = [&bar](UI::Table& hint){
									auto canvas = UI::makeCanvas([&bar](const UI::Elem& elem){
										using namespace Graphic;
										using Draw::Overlay;

										Geom::Vec2 offset{};

										const auto progress = getSizeFromProgress(bar.getProgress());
										const auto [srcX, srcY] = Align::getOffsetOf(Align::Layout::center,
											progress.as<float>().scl(
												elem.getValidWidth() / MaxTileSize,
												elem.getValidHeight() / MaxTileSize),
											elem.getValidBound().copy().move(elem.getAbsSrc())
										);
										Overlay::Line::setLineStroke(2.f);
										Overlay::color(UI::Pal::LIGHT_GRAY, elem.maskOpacity);

										for(int x = 0; x < progress.x; ++x){
											for(int y = 0; y < progress.y; ++y){
												Overlay::Line::rectOrtho(
													srcX + offset.x,
													srcY + offset.y,
													elem.getValidWidth() / MaxTileSize,
													elem.getValidHeight() / MaxTileSize
												);

												offset.y += elem.getValidHeight() / MaxTileSize;
											}

											offset.x += elem.getValidWidth() / MaxTileSize;
											offset.y = 0.f;
										}
									});

									canvas->setEmptyDrawer();

									hint.transferElem(std::move(canvas)).setSize(300.f, 300.f, true).endLine();

									hint.add<UI::Label>([&bar](UI::Label& label){
										label.setEmptyDrawer();
										label.setWrap(false, true);
										label.setTextAlign(Align::Layout::center_left);
										label.setText([&bar]{
											auto [x, y] = getSizeFromProgress(bar.getProgress());
											return std::format(" {}x{} ", x, y);
										});
										label.setTextScl(0.7f);
									}).fillParentX().wrapY();
								}
							});
					}).fillParentX().setHeight(300.0f).endLine().as<UI::SliderBar>();

					table.add<UI::Button>([this](UI::Button& button){
						button.setCellAlignMode(Align::Layout::left);
						auto& imageRegion = button.add<UI::ImageRegion>([this](UI::ImageRegion& image){
							image.setEmptyDrawer();
							image.scaling = Align::Scale::fit;
							image.setDrawable(UI::Icons::iconMap.at(ext::getBoolOpName(booleanOperation)).get());
						}).setSize(60.f).as<UI::ImageRegion>();

						button.add<UI::Label>([this](UI::Label& label){
							label.setEmptyDrawer();
							label.setText([this]{
								return std::string(getBundleEntry(ext::getBoolOpName(booleanOperation)));
							});
							label.setTextAlign(Align::Layout::top_left);
							label.setTextScl(0.4f);
						}).fillParent().setMargin({.left = 3.f});

						button.setTooltipBuilder({
								.followTarget = UI::TooltipBuilder::FollowTarget::parent,
								.minHoverTime = UI::DisableAutoTooltip,
								.builder = [this, &imageRegion](UI::Table& section){
									section.setCellAlignMode(Align::Layout::center);
									for(auto [op, name] : ext::AllNamedBoolOp){
										section.add<UI::Button>([op, name, this, &imageRegion](UI::Button& select){
											select.setDrawer(UI::Styles::drawer_elem_s1_noEdge.get());
											select.add<UI::ImageRegion>([op, name, this](UI::ImageRegion& image){
												image.setEmptyDrawer();
												image.scaling = Align::Scale::fit;
												image.setDrawable(UI::Icons::iconMap.at(name).get());
											}).setSize(64, 64);

											select.setCall([this, op, &imageRegion](auto&){
												booleanOperation = op;
												imageRegion.setDrawable(UI::Icons::iconMap.at(ext::getBoolOpName(booleanOperation)).get());
											});

											select.setActivatedChecker([this, op](UI::Elem& select_1){
												if(booleanOperation == op){
													select_1.color = UI::Pal::PALE_GREEN;
													return true;
												} else{
													select_1.color = UI::Pal::WHITE;
													return false;
												}
											});
										}).expand().setPad({.left = 3.f, .right = 3.f});
									}
								}
							});

						button.setCall([](UI::Button& b){
							b.buildTooltip();
						});
					}).wrapY().fillParentX().endLine();
				});
			}).setSizeScale(0.2f, 0.9f).setMargin(4.0f).setAlign(Align::Layout::top_left);

			UI::Table::add<UI::Button>([this](UI::Button& button){
				// button.add<UI::Label>([](UI::Label& label){
				// 	label.setText(label.getBundleEntry("exit", true));
				// 	label.setEmptyDrawer();
				// 	label.setTextAlign(Align::center);
				// });
				// button.setCall([this](UI::Button& b, bool){
				// 	if(tryEsc())destroy();
				// });
			}).setSizeScale(0.2f, 0.1f).setMargin(4.0f).setAlign(Align::Layout::bottom_left);

			screen = &UI::Table::add<UI::Screen>([](UI::Screen& screen){
				screen.usesUIEffect = true;
			}).setSizeScale(0.8f, 1.0f).setMargin(4.0f).setAlign(Align::Layout::right).as<UI::Screen>();
		}

	public:
		[[nodiscard]] ChamberWorkshopBase(){
			setLayoutByRelative(false);
		}

		UI::CursorType getCursorType() const noexcept override{
			if(touchbility != UI::TouchbilityFlags::enabled || !screen->isInbound(getCursorPos())){
				return tooltipbuilder ? UI::CursorType::regular_tip : UI::CursorType::regular;
			} else{
				return tooltipbuilder ? UI::CursorType::clickable_tip : UI::CursorType::clickable;
			}
		}
	};

	// using T = int;
	export
	template <typename T>
	class ChamberWorkshop : public ChamberWorkshopBase{
		using Tile = ChamberTile<T>;
		using TileGrid = ChamberGridData<T>;
		using EntityType = T;

		//TODO maybe shared_ptr is better?
		TileGrid grid{};
		std::unordered_map<Geom::Point2, typename TileGrid::ItrType> selected{};

	public:
		ChamberFactory<T>* currentFactory{};

		void build(const ChamberGridData<T>& data){
			grid = data;

			buildElem();

			screen->getInputListener().on<UI::MouseActionPress>([this](const UI::MouseActionPress& e){
				if(e.mode == Ctrl::Mode::None)
					switch(e.key){
						case Ctrl::Mouse::_2 :{
							if(!currentFactory) break;
							const auto pos = toChamberPos(screen->getCursorPosInScreen());
							Geom::OrthoRectInt region = getTileBound();
							region.setCenter(pos);

							if(grid.placementValid(region)){
								bool contains{false};
								region.each([this, &contains](const Geom::Point2 p){
									if(contains)return;
									if(selected.erase(p))contains = true;
								});

								grid.insert(currentFactory->genChamberTile(region));

								if(contains)region.each([this](const Geom::Point2 p){
									if(const auto itr = grid.findItr(p); itr != grid.getData().end()){
										selected.insert_or_assign(p, itr);
									}
								});
							}
							break;
						}

						case Ctrl::Mouse::_1 :{
							const auto pos = toChamberPos(screen->getCursorPosInScreen());
							const auto region = grid.erase(pos, true);

							region.each([this](const Geom::Point2 p) {
								selected.erase(p);
							});

							break;
						}

						default : break;
					}

				if(e.key == Ctrl::Mouse::_1 && e.mode == Ctrl::Mode::Ctrl){
					if(!boxSelection.isSelecting())
						boxSelection.beginSelect(
							toChamberPos(screen->getCursorPosInScreen()));
				}
			});

			screen->getCamera().setScaleRange(0.5f, 10.f);

			screen->getInputListener().on<UI::MouseActionRelease>([this](const UI::MouseActionRelease& e){
				if(e.key == Ctrl::Mouse::_1){
					if(boxSelection)
						boxSelection.endSelect(toChamberPos(screen->getCursorPosInScreen()),
							[this](const decltype(boxSelection)::Rect region){
								std::unordered_map<Geom::Point2, typename TileGrid::ItrType>
									newSelected{};

								region.each([this, &newSelected](Geom::Point2 pos){
									if(const auto itr = grid.findItr(pos); itr != grid.getData().end()){
										newSelected.try_emplace(pos, itr);

										if(itr->valid()){
											const auto chamberRegion = itr->getChamberGridBound();

											chamberRegion.each([this, &newSelected](Geom::Point2 p){
												if(const auto itr2 = grid.findItr(p); itr2 != grid.getData().end()){
													newSelected.try_emplace(p, itr2);
												}
											});
										}
									}
								});

								ext::booleanConj(booleanOperation, selected, std::move(newSelected));
							});
				}
			});
		}

		void drawContent() const override{
			Table::drawContent();

			using namespace Graphic;
			using Draw::Overlay;

			screen->beginDraw(&Core::BatchGroup::overlay);
			const auto& brief = grid.getBrief();

			{
				GL::UniformGuard _{Assets::Shaders::coordAxisArgs, &screen->getCamera()};
				Graphic::Mesh::meshBegin();
				Graphic::Mesh::meshEnd(true, Assets::Shaders::coordAxis);
			}

			//TODO is screen space clip necessary?
			if(Core::renderer){
				screen->unlockViewport();

				Core::renderer->effectBuffer.bind();
				Core::renderer->effectBuffer.enableDrawAll();
				Core::renderer->effectBuffer.clearColorAll();
				Overlay::color(Colors::DARK_GRAY);

				for(const Tile* tile : brief.invalids){
					if(!screen->getCamera().getViewport().overlap(tile->getBound())) continue;
					Overlay::Fill::rectOrtho(Overlay::getContextTexture(), tile->getBound());
				}

				[[maybe_unused]] GL::UniformGuard guard_outline
					{
						Assets::Shaders::outlineArgs, 1.2f * screen->getCamera().getScale(), 0.f,
						screen->getCamera().getScreenSize().inverse()
					};
				screen->getBatch()->flush();

				Core::renderer->effectBuffer.getTexture().active(0);
				Frame::blit(&screen->getBuffer(), 0, Assets::Shaders::outline_ortho);
				screen->lockViewport();
			}

			{
				Overlay::color(Colors::WHITE, 0.6f);
				Overlay::mixColor(Colors::BLACK.createLerp(Colors::RED_DUSK, 0.3f));

				[[maybe_unused]] GL::UniformGuard guard_slideLine_1{
						Assets::Shaders::slideLineShaderDrawArgs, 25.0f, 45.0f, Colors::CLEAR
					};
				[[maybe_unused]] GL::UniformGuard guard_slideLine_2{
						Assets::Shaders::slideLineShaderScaleArgs, 1.f, screen->getCamera().getScreenSize()
					};
				[[maybe_unused]] Core::BatchGuard_Shader guard_batchShader{
						*Core::batchGroup.overlay, Assets::Shaders::sildeLines
					};

				for(const Tile* tile : brief.invalids){
					if(!screen->getCamera().getViewport().overlap(tile->getChamberRegion())) continue;
					Overlay::Fill::rectOrtho(Overlay::getContextTexture(), tile->getBound());
				}
			}

			Overlay::mixColor();

			Overlay::color(Colors::GRAY, 0.45f);
			Overlay::Line::setLineStroke(1.0f);
			for(const Tile* tile : brief.valids){
				if(!screen->getCamera().getViewport().overlap(tile->getBound())) continue;
				Overlay::Line::rectOrtho(tile->getBound());
			}

			Overlay::Line::setLineStroke(1.0f);
			Overlay::color(Colors::LIGHT_GRAY, 0.85f);
			for(const auto tile : brief.owners){
				if(!screen->getCamera().getViewport().overlap(tile->getEntityBound())) continue;
				Overlay::Line::rectOrtho(tile->getEntityBound());
			}

			Overlay::color(UI::Pal::THEME.createLerp(Colors::WHITE, Math::absin(OS::globalTime(), 0.83f, 0.53f)));
			for(const Tile& tile : selected | std::ranges::views::values | std::ranges::views::transform(
				    &TileGrid::ItrType::operator*)){
				const auto region = tile.getChamberRegion();

				if(!screen->getCamera().getViewport().overlap(region)) continue;

				if(tile.valid()){
					const float minS = Math::min(region.getWidth(), region.getHeight());
					Overlay::Line::setLineStroke(2.2f);
					Overlay::Fancy::select_rectOrtho(region, minS * 0.285f, true, 0.5f);
				}else{
					Overlay::Line::setLineStroke(0.68f);
					Overlay::Fancy::select_rectOrtho(region, 3.f);
				}

			}

			const auto pos = toChamberPos(screen->getCursorPosInScreen());
			Geom::OrthoRectInt bound = getTileBound();
			bound.setCenter(pos);

			if(const auto finded = grid.find(pos)){
				Overlay::color(Colors::LIGHT_GRAY);
				Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), finded->getBound());
			}

			if(grid.placementValid(bound)){
				Overlay::color(Colors::PALE_GREEN);
			} else{
				Overlay::color(Colors::RED_DUSK);
			}

			Overlay::Line::setLineStroke(2.f);
			Overlay::Line::rectOrtho(bound.as<float>().scl(TileSize, TileSize));

			if(boxSelection){
				const auto region = boxSelection.getRegion(toChamberPos(screen->getCursorPosInScreen())).as<float>().scl(
						TileSize, TileSize);
				Overlay::color(UI::Pal::THEME);
				if(region.area() == 0.f){
					Overlay::Line::line(region.getSrc(), region.getEnd());
				}else{
					Overlay::Line::rectOrtho(region);
				}
			}

			screen->endDraw();
		}

		[[nodiscard]] const TileGrid& getFrame() const{ return grid; }

		[[nodiscard]] TileGrid& getFrame(){ return grid; }
	};
}
