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

export import Game.Chamber;
export import Game.Chamber.Frame;

import Core;
import Graphic.Draw;
import Assets.Graphic;
import GL.Shader.UniformWrapper;

import ext.BooleanOperation;

export namespace Game::Scene{

	template <typename T>
	struct ChamberWorkshopMenuBuilder{
		std::vector<ChamberFactory<T>*> getFactories() = delete; 
	};

	template <typename T>
	// using T = int;
	class ChamberWorkshop : public UI::Table{
		static constexpr auto MaxTileSize = 10;

		static Geom::Point2 getSizeFromProgress(Geom::Vec2 progress){
			return progress.scl(MaxTileSize - 1).round<int>().add(1, 1);
		}

		using Tile = ChamberTile<T>;
		using TileFrame = ChamberGridData<T>;

		//TODO maybe shared_ptr is better?
		TileFrame frame{};
		std::unordered_map<Geom::Point2, typename TileFrame::ItrType> selected{};

		UI::Screen* screen{};
		UI::Table* selectionTable{};
		UI::SliderBar* slider{};

		Geom::Point2 selectionBegin{Geom::maxVec2<int>};

		enum struct ChamberFilter{
			owner,
			invalid
		};

		ChamberFilter filterType{};
		ext::BooleanOperation booleanOperation{};

		constexpr bool isBoxSelecting() const noexcept{
			return selectionBegin != Geom::maxVec2<int>;
		}

		void boxSelectBegin(){
			selectionBegin = toChamberPos(screen->getCursorPosInScreen());
		}

		Geom::OrthoRectInt getSelectionRange() const noexcept{
			Geom::OrthoRectInt selectionRange{};
			selectionRange.setVert(selectionBegin, toChamberPos(screen->getCursorPosInScreen()));

			return selectionRange;
		}

		void boxSelectEnd(){
			auto selectionRange = getSelectionRange();
			std::unordered_map<Geom::Point2, typename TileFrame::ItrType> newSelected{};

			selectionRange.each([this, &newSelected](Geom::Point2 pos){
				if(const auto itr = frame.findItr(pos); itr != frame.getData().end()){
					newSelected.try_emplace(pos, itr);
				}
			});

			ext::booleanConj(booleanOperation, selected, std::move(newSelected));

			selectionBegin = Geom::maxVec2<int>;
		}

		Geom::OrthoRectInt getTileBound() const{
			if(slider){
				auto [w, h] = getSizeFromProgress(slider->getProgress());
				return Geom::OrthoRectInt{w, h};
			}else [[unlikely]] {
				return {};
			}
		}

		void buildElem(){
			setEmptyDrawer();

			UI::Table::add<UI::ScrollPane>([this](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([this](Table& table){
					table.defaultCellLayout.fillParentX();
					table.defaultCellLayout.setPad({.bottom = 6.f});

					table.setEmptyDrawer();
					{
						UI::LayoutCell& cell = table.add<UI::SliderBar>([](UI::SliderBar& bar){
							bar.setSegments(Geom::norBaseVec2<unsigned>.copy().scl(MaxTileSize - 1));

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
										UI::LayoutCell& c = hint.transferElem(std::move(canvas));
										c.setSize(300.f, 300.f, true).endLine();

										UI::LayoutCell& c1 = hint.add<UI::Label>([&bar](UI::Label& label){
											label.setEmptyDrawer();
											label.setWrap(false, true);
											label.setTextAlign(Align::Layout::center_left);
											label.setText([&bar]{
												auto [x, y] = getSizeFromProgress(bar.getProgress());
												return std::format(" {}x{} ", x, y);
											});
											label.setTextScl(0.7f);
										});
										c1.fillParentX().wrapY();
									}
								});
						});
						cell.fillParentX().setHeight(300.0f).endLine();
						slider = &cell.as<UI::SliderBar>();
					}

					{
						UI::LayoutCell& cell = table.add<UI::Button>([this](UI::Button& button){
							button.setTooltipBuilder({
									.followTarget = UI::TooltipBuilder::FollowTarget::parent,
									.minHoverTime = UI::DisableAutoTooltip,
									.builder = [this](UI::Table& section){
										section.setCellAlignMode(Align::Layout::center);
										for(auto [op, name] : ext::AllNamedBoolOp){
											section.add<UI::Button>([=, this](UI::Button& select){
												select.add<UI::ImageRegion>([=, this](UI::ImageRegion& image){
													image.setEmptyDrawer();
													image.scaling = Align::Scale::fit;
													image.setDrawable(UI::Icons::iconMap.at(name).get());
												}).setSize(64, 64);

												select.setCall([this, op](auto&, bool){
													booleanOperation = op;
												});

												select.setActivatedChecker([this, op, &select]{
													if(booleanOperation == op){
														select.color = UI::Pal::PALE_GREEN;
														return true;
													} else{
														select.color = UI::Pal::WHITE;
														return false;
													}
												});
											}).expand().setPad({.left = 3.f, .right = 3.f});
										}
									}
								});

							button.setCall([](UI::Button& b, bool){
								b.buildTooltip();
							});
						});

						cell.setHeight(60.f).endLine();
					}
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
			}).setSizeScale(0.8f, 1.0f).setMargin(4.0f).setAlign(Align::Layout::right).template as<UI::Screen>();

			screen->getInputListener().on<UI::MouseActionPress>([this](const UI::MouseActionPress& e){
				if(e.mode == Ctrl::Mode::None)switch(e.key){
					case Ctrl::Mouse::_2 :{
						if(!currentFactory)break;
						const auto pos = toChamberPos(screen->getCursorPosInScreen());
						Geom::OrthoRectInt bound = getTileBound();
						bound.setCenter(pos);

						if(frame.placementValid(bound)){
							frame.insert(currentFactory->genChamberTile(bound));
						}
						break;
					}

					case Ctrl::Mouse::_1 :{
						const auto pos = toChamberPos(screen->getCursorPosInScreen());
						frame.erase(pos, true);
						break;
					}

					default: break;
				}

				if(e.key == Ctrl::Mouse::_1 && e.mode == Ctrl::Mode::Ctrl){
					if(!this->isBoxSelecting())boxSelectBegin();
				}
			});

			screen->getInputListener().on<UI::MouseActionRelease>([this](const UI::MouseActionRelease& e){
				if(e.key == Ctrl::Mouse::_1){
					if(isBoxSelecting())boxSelectEnd();
				}
			});
		}
	public:

		ChamberFactory<T>* currentFactory{};

		using EntityType = T;

		[[nodiscard]] ChamberWorkshop(){
			setLayoutByRelative(false);
		}

		void build(const ChamberGridData<T>& data){
			frame = data;

			buildElem();
		}

		void drawContent() const override{
			Table::drawContent();

			using namespace Graphic;
			using Draw::Overlay;

			screen->beginDraw(&Core::BatchGroup::overlay);
			const auto& brief = frame.getBrief();

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
				Overlay::color(Colors::GRAY);

				for(const Tile& tile : brief.invalids){
					if(!screen->getCamera().getViewport().overlap(tile.getTileBound()))continue;
					Overlay::Fill::rectOrtho(Overlay::getContextTexture(), tile.getTileBound());
				}

				[[maybe_unused]] GL::UniformGuard guard_outline
					{
						Assets::Shaders::outlineArgs, 2.0f * screen->getCamera().getScale(), 0.f,
						screen->getCamera().getScreenSize().inverse()
					};
				screen->getBatch()->flush();

				// GL::enable(GL::State::BLEND);
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

				for(const Tile& tile : brief.invalids){
					if(!screen->getCamera().getViewport().overlap(tile.getChamberRegion()))continue;
					Overlay::Fill::rectOrtho(Overlay::getContextTexture(), tile.getTileBound());
				}
			}

			Overlay::mixColor();
			Overlay::color(Colors::GRAY, 0.45f);
			Overlay::Line::setLineStroke(1.0f);
			for(const Tile& tile : brief.valids){
				if(!screen->getCamera().getViewport().overlap(tile.getTileBound()))continue;
				Overlay::Line::rectOrtho(tile.getTileBound());
			}

			Overlay::Line::setLineStroke(1.0f);
			Overlay::color(Colors::LIGHT_GRAY, 0.85f);
			for(const auto tile : brief.owners){
				if(!screen->getCamera().getViewport().overlap(tile->getEntityBound()))continue;
				Overlay::Line::rectOrtho(tile->getEntityBound());
			}

			Overlay::Line::setLineStroke(0.68f);
			Overlay::color(Colors::AQUA_SKY, 0.85f);
			for(const Tile& tile : selected | std::ranges::views::values | std::ranges::views::transform(&TileFrame::ItrType::operator*)){
				if(!screen->getCamera().getViewport().overlap(tile.getTileBound()))continue;
				Overlay::Fancy::select_rectOrtho(tile.getTileBound(), 3.f);
			}

			const auto pos = toChamberPos(screen->getCursorPosInScreen());
			Geom::OrthoRectInt bound = getTileBound();
			bound.setCenter(pos);

			if(const auto finded = frame.find(pos)){
				Overlay::color(Colors::LIGHT_GRAY);
				Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), finded->getTileBound());
			}

			if(frame.placementValid(bound)){
				Overlay::color(Colors::PALE_GREEN);
			} else{
				Overlay::color(Colors::RED_DUSK);
			}

			Overlay::Line::setLineStroke(2.f);
			Overlay::Line::rectOrtho(bound.as<float>().scl(TileSize, TileSize));

			if(isBoxSelecting()){
				Overlay::color(Colors::AQUA_SKY);
				Overlay::Line::rectOrtho(getSelectionRange().as<float>().scl(TileSize, TileSize));
			}

			screen->endDraw();
		}

		UI::CursorType getCursorType() const noexcept override{
			if(touchbility != UI::TouchbilityFlags::enabled || !screen->isInbound(getCursorPos())){
				return tooltipbuilder ? UI::CursorType::regular_tip : UI::CursorType::regular;
			}else{
				return tooltipbuilder ? UI::CursorType::clickable_tip : UI::CursorType::clickable;
			}
		}

		[[nodiscard]] const TileFrame& getFrame() const{ return frame; }
		[[nodiscard]] TileFrame& getFrame(){ return frame; }
	};
}
