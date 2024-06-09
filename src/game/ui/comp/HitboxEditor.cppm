//
// Created by Matrix on 2024/6/5.
//

export module Game.UI.HitBoxEditor;

import UI.Table;
import UI.Screen;
import UI.ImageRegion;
import UI.RegionDrawable;
import UI.Icons;
import UI.Styles;
import UI.Creation;
import UI.Selection;
import UI.Palette;
import std;

import Geom.Shape.RectBox;
import Geom.QuadTree;

import Game.Entity.Collision;

import Graphic.Draw;
import Game.Graphic.Draw;
import Assets.Graphic;

import OS.Ctrl.Bind;
import OS.Ctrl.Operation;

import ext.bool_merge;

namespace Game::Scene{
	export class HitboxEditor : public UI::Table{
		static constexpr Geom::OrthoRectFloat MaximumBound{-20000, -20000, 40000, 40000};

		inline static OS::InputBindGroup editorBinds{};

		struct EditBox : Geom::QuadTreeAdaptable<EditBox>{
			HitBoxFragmentData staticData{};
			HitBoxFragmentData drawData{};

			bool intersected{};
			bool selected{};

			[[nodiscard]] constexpr Geom::OrthoRectFloat getBound() const noexcept{
				return drawData.boxData.maxOrthoBound;
			}

			[[nodiscard]] constexpr bool roughIntersectWith(const EditBox& other) const{
				return drawData.boxData.overlapRough(other.drawData.boxData);
			}

			[[nodiscard]] constexpr bool exactIntersectWith(const EditBox& other) const{
				return drawData.boxData.overlapExact(other.drawData.boxData);
			}

			[[nodiscard]] bool containsPoint(const Geom::Vec2 point) const noexcept{
				return drawData.boxData.contains(point);
			}

			void draw() const{
				if(selected){
					Graphic::Draw::Overlay::color(UI::Pal::KEY_WORD);
				} else if(intersected){
					Graphic::Draw::Overlay::color(UI::Pal::THEME);
				} else{
					Graphic::Draw::Overlay::color(UI::Pal::LIGHT_GRAY);
				}

				Graphic::Draw::Overlay::alpha(0.5f);

				Game::Draw::hitbox<Graphic::Draw::Overlay>(drawData.boxData);

				Graphic::Draw::Overlay::color(UI::Pal::GRAY, 0.5f);
				Graphic::Draw::Overlay::Line::setLineStroke(1.f);
				Graphic::Draw::Overlay::Line::rectOrtho(drawData.boxData.maxOrthoBound);
			}

			void drawLocalAxis(const Geom::Vec2 clamp, const float ang) const{
				Graphic::Draw::Overlay::Line::setLineStroke(2.0f);
				if(!Math::zero(clamp.x)){
					Graphic::Draw::Overlay::color(Graphic::Colors::RED_DUSK);
					Graphic::Draw::Overlay::Line::lineAngleCenter(drawData.boxData.originPoint.x,
						drawData.boxData.originPoint.y,
						ang, 50000);
				}

				if(!Math::zero(clamp.y)){
					Graphic::Draw::Overlay::color(Graphic::Colors::ACID);
					Graphic::Draw::Overlay::Line::lineAngleCenter(drawData.boxData.originPoint.x,
						drawData.boxData.originPoint.y,
						ang + 90, 50000);
				}
			}
		};

		enum class EditOperation{
			none,
			cancel,
			moveTrans,
			scale,
			rotate,
		};

		enum class BoxSelectFunc{
			reallocate,
			intersecton,
			combine,
			sub,
		};

		enum class SpaceTransType{
			screenSpace,
			originSpace,
			localSpace
		};

		UI::BoxSelection<float> boxSelection{};

		using TreeType = Geom::QuadTree<EditBox, float, false>;

		std::vector<EditBox> boxes{};
		TreeType quadTree{MaximumBound};
		std::unordered_set<EditBox*> selected{};

		UI::Table* sideMenu{};
		UI::Screen* screen{};

		ext::algo::merge_policy mergePolicy = ext::algo::merge_policy::replace;
		SpaceTransType transType = SpaceTransType::localSpace;
		EditOperation curOp = EditOperation::none;
		BoxSelectFunc boxSelectFunc = BoxSelectFunc::reallocate;

		bool movingOrigin{};
		Geom::Transform originTrans{};

		float operationAccuracy = 1.0f;
		Geom::Vec2 clamp{1, 1};

		void setOriginTrans_sustainLocal(const Geom::Transform transform){
			for(auto& box : boxes){
				box.staticData.localTrans |= originTrans;
				box.staticData.localTrans.applyInv(transform);
			}

			originTrans = transform;
		}

		void addSelection(EditBox* entity){
			selected.insert(entity);
			entity->selected = true;
		}

		void eraseSelection(EditBox* entity){
			selected.erase(entity);
			entity->selected = false;
		}

		void selectAll(){
			for(auto& entity : boxes){
				this->addSelection(&entity);
			}
		}

		void clearSelected(){
			for(auto& entity : boxes){
				entity.selected = false;
			}

			selected.clear();
		}

		void buildBasic(){
			using namespace UI;
			sideMenu =
				&add<Table>([this](Table& sideMenu){
					 sideMenu.setFillparent();
					 sideMenu.setCellAlignMode(Align::Layout::top_center);
					 sideMenu.defaultCellLayout.setPad({.bottom = 4.f, .top = 4.f});

					 sideMenu.add<UI::Table>([this](UI::Table& transTable){
						 transTable.setEmptyDrawer();

						 std::array modes{
								 std::pair{SpaceTransType::screenSpace, Icons::abnormal},
								 std::pair{SpaceTransType::localSpace, Icons::abnormal},
								 std::pair{SpaceTransType::originSpace, Icons::abnormal},
							 };

						 for(const auto& [idx, data] : modes | std::views::enumerate){
							 transTable.transferElem(Create::imageButton(Styles::drawer_elem_s1_noEdge.get(),
								 data.second, [this, mode = data.first]{
									 transType = mode;
								 }, [this, mode = data.first](Button& b){

									 b.setActivatedChecker(Create::ValueChecker<&HitboxEditor::transType>{this, mode});
								 }
							 ).first).fillParent();


							 if(idx & 1){
								 transTable.endline();
							 }
						 }


					 }).fillParentX().setYRatio(1.f);

					sideMenu.endline().emplace(Create::LineCreater{}).setHeight(4.f).fillParentX().endLine();

				 })
				 .setWidth(120.f)
				 .fillParentY()
				 .as<Table>();

			screen = &add<UI::Screen>([](UI::Screen& section){
				// section.setEmptyDrawer();
			}).fillParent().as<UI::Screen>();

			screen->getInputListener().on<UI::MouseActionPress>([this](const UI::MouseActionPress& e){
				EditBox* hit{};

				if(hasOp() && e.key == Ctrl::Mouse::LMB){
					endOp();
					return;
				}

				switch(e.key){
					case Ctrl::Mouse::_2 :{
						quadTree.intersectPoint(screen->getCursorPosInScreen(), [&hit](EditBox& b){
							if(!hit && b.selected) hit = &b;
						});
						if(hit) eraseSelection(hit);
						break;
					}

					case Ctrl::Mouse::_1 :{
						if(e.mode == Ctrl::Mode::Ctrl) break;
						quadTree.intersectPoint(screen->getCursorPosInScreen(), [&hit](EditBox& b){
							if(!hit && !b.selected) hit = &b;
						});
						if(hit) addSelection(hit);
						break;
					}

					default : break;
				}
			});
		}

		void switchY(){
			if(clamp.x < 1){
				resumeX();
			} else{
				limitX();
				resumeY();
			}
		}

		void switchX(){
			if(clamp.y < 1){
				resumeY();
			} else{
				limitY();
				resumeX();
			}
		}

		void limitX(){ clamp.x = 0; }

		void limitY(){ clamp.y = 0; }

		void resumeX(){ clamp.x = 1; }

		void resumeY(){ clamp.y = 1; }

		void resetOp(){
			curOp = EditOperation::none;

			resumeX();
			resumeY();

			boxSelection.setInvalid();
		}

		void endOp(){
			for(const auto& entity : selected){
				entity->staticData = entity->drawData;
			}

			resetOp();
		}

		void cancelOp(){
			for(const auto& entity : selected){
				entity->drawData = entity->staticData;
			}

			resetOp();
		}

		void switchOp(const EditOperation op){
			boxSelection.src = screen->getCursorPosInScreen();

			curOp = op;
			switch(op){
				case EditOperation::cancel : cancelOp();
					break;
				case EditOperation::none : endOp();
				default : break;
			}
		}

		void processMove() const{
			for(auto* entity : selected){
				Geom::Vec2 dst{(screen->getCursorPosInScreen() - boxSelection.src).scl(operationAccuracy)};

				float ang = -originTrans.rot;

				if(clamp.area() == 0){
					switch(transType){
						case SpaceTransType::localSpace : ang = entity->drawData.localTrans.rot;
							break;
						case SpaceTransType::originSpace : ang = 0;
							break;
						case SpaceTransType::screenSpace : ang = -originTrans.rot;
							break;
						default : break;
					}

					dst.project(clamp.copy().rotate(ang));
				} else{
					dst.rotate(ang);
				}


				entity->drawData.localTrans = entity->staticData.localTrans;
				entity->drawData.localTrans.vec += dst;

				entity->drawData.boxData.update(entity->drawData.localTrans);
			}
		}

		void processScale() const{
			float dst = Math::max(-15.0f,
				(screen->getCursorPosInScreen() - boxSelection.src).scl(operationAccuracy).x * 0.05f);
			if(dst < 0){
				dst = Math::curve(dst, -15.0f, 0.0f) - 1.0f;
			}
			for(const auto& entity : selected){
				entity->drawData.boxData = entity->staticData.boxData;

				const Geom::Vec2 scl = {1 + dst * clamp.x, 1 + dst * clamp.y};
				entity->drawData.boxData.offset *= scl;
				entity->drawData.boxData.sizeVec2 *= scl;

				entity->drawData.boxData.update(entity->drawData.localTrans);
			}
		}

		void processRotate() const{
			for(const auto& entity : selected){
				const float ang1 = (boxSelection.src - entity->drawData.localTrans.vec).angle();
				const float ang2 = (screen->getCursorPosInScreen() - entity->drawData.localTrans.vec).angle();
				const float dst = Math::Angle::angleDstWithSign(ang1, ang2);

				entity->drawData = entity->staticData;
				entity->drawData.localTrans.rot += dst;
				entity->drawData.boxData.update(entity->staticData.localTrans);
			}
		}

		void resetTransMove() const{
			for(const auto& entity : selected){
				entity->staticData.localTrans.vec.setZero();
				entity->drawData.boxData.update(entity->drawData.localTrans);
			}
		}

		void resetRotate() const{
			for(const auto& entity : selected){
				entity->staticData.localTrans.rot = 0;
				entity->drawData.boxData.update(entity->staticData.localTrans);
			}
		}

		void deleteSelected(){
			std::erase_if(boxes, [this](auto& box){
				return selected.contains(&box);
			});

			selected.clear();
		}

		bool hasOp() const{
			return std::to_underlying(curOp) > 1;
		}

		void loadBinds(){
			::Ctrl::Operation boxSelectBegin{
					"boxSelectBegin", OS::KeyBind(Ctrl::Mouse::_1, Ctrl::Act::Press, Ctrl::Mode::Ctrl, [this]{
						if(!this->boxSelection.isSelecting()){
							endOp();
							boxSelection.beginSelect(screen->getCursorPosInScreen());
						}
					})
				};

			::Ctrl::Operation boxSelectEnd{
					"boxSelectEnd", OS::KeyBind(Ctrl::Mouse::_1, Ctrl::Act::Release, [this]{
						if(boxSelection){
							boxSelection.endSelect(screen->getCursorPosInScreen(), [this](const UI::Rect rect){
								//TODO apply binary op
								clearSelected();

								quadTree.intersectRect(rect, [this](EditBox& box, const UI::Rect& r){
									if(box.drawData.boxData.overlapRough(r) && box.drawData.boxData.overlapExact(r)){
										addSelection(&box);
									}
								});
							});
						}
					})
				};

			::Ctrl::Operation add{
					"add", OS::KeyBind(Ctrl::Key::A, Ctrl::Act::Press, Ctrl::Mode::Shift, [this]{
						auto& editBox = boxes.emplace_back();

						clearSelected();

						editBox.staticData.localTrans.vec = screen->getCursorPosInScreen();
						editBox.staticData.localTrans.applyInv(originTrans);

						Geom::RectBox& box{editBox.staticData.boxData};
						box.setSize(60, 60);
						box.offset.set(box.sizeVec2).scl(-0.5f);

						editBox.drawData = editBox.staticData;

						addSelection(&editBox);
					})
				};

			::Ctrl::Operation deleteSelection{
					"deleteSelection", OS::KeyBind(Ctrl::Key::Delete, Ctrl::Act::Press, [this]{
						deleteSelected();
					})
				};

			::Ctrl::Operation selectAllBox{
					"selectAll", OS::KeyBind(Ctrl::Key::A, Ctrl::Act::Press, Ctrl::Mode::NoIgnore, [this]{
						selectAll();
					})
				};

			::Ctrl::Operation rotate{
					"rotate", OS::InputBind(Ctrl::Key::R, Ctrl::Act::Press, Ctrl::Mode::NoIgnore, [this]{
						switchOp(EditOperation::rotate);
					})
				};

			::Ctrl::Operation scale{
					"scale", OS::InputBind(Ctrl::Key::S, Ctrl::Act::Press, Ctrl::Mode::NoIgnore, [this]{
						switchOp(EditOperation::scale);
					})
				};

			::Ctrl::Operation move{
					"move", OS::InputBind(Ctrl::Key::G, Ctrl::Act::Press, Ctrl::Mode::NoIgnore, [this]{
						switchOp(EditOperation::moveTrans);
					})
				};

			::Ctrl::Operation clearRot{
					"clearRot", OS::InputBind(Ctrl::Key::R, Ctrl::Act::Press, Ctrl::Mode::Alt, [this]{
						if(selected.size() == 1) resetRotate();
					})
				};

			::Ctrl::Operation clearMove{
					"clearMove", OS::InputBind(Ctrl::Key::G, Ctrl::Act::Press, Ctrl::Mode::Alt, [this]{
						if(selected.size() == 1) resetTransMove();
					})
				};

			::Ctrl::Operation clearSelection{
					"clearSelection", OS::InputBind(Ctrl::Key::Esc, Ctrl::Act::Press, [this]{
						if(this->hasOp()) this->switchOp(EditOperation::cancel);
						else this->clearSelected();
					})
				};

			::Ctrl::Operation confirm{
					"confirm", OS::InputBind(Ctrl::Key::Enter, Ctrl::Act::Press, [this]{
						this->endOp();
					})
				};

			::Ctrl::Operation clampX{
					"clampX", OS::InputBind(Ctrl::Key::X, Ctrl::Act::Press, Ctrl::Mode::NoIgnore, [this]{
						this->switchX();
					})
				};

			::Ctrl::Operation clampY{
					"clampY", OS::InputBind(Ctrl::Key::Y, Ctrl::Act::Press, Ctrl::Mode::NoIgnore, [this]{
						this->switchY();
					})
				};

			::Ctrl::Operation accurate_prs{
					"accurate-prs", OS::InputBind(Ctrl::Key::Left_Shift, Ctrl::Act::Press, [this]{
						this->operationAccuracy = 0.075f;
					})
				};

			::Ctrl::Operation accurate_rls{
					"accurate-rls", OS::InputBind(Ctrl::Key::Left_Shift, Ctrl::Act::Release, [this]{
						this->operationAccuracy = 1.0f;
					}),
					{accurate_prs.name}
				};

			::Ctrl::OperationGroup basicGroup{
					"basic-group", {
						std::move(boxSelectBegin),
						std::move(boxSelectEnd),
						std::move(selectAllBox),
						std::move(add),
						std::move(deleteSelection),
						std::move(rotate),
						std::move(scale),
						std::move(move),
						std::move(clearRot),
						std::move(clearMove),
						std::move(clearSelection),
						std::move(confirm),
						std::move(clampX),
						std::move(clampY),
						std::move(accurate_prs),
						std::move(accurate_rls),
					}
				};

			basicGroup.targetGroup = &editorBinds;
			basicGroup.applyToTarget();

			loadInputBinds(editorBinds);
		}

	public:
		explicit HitboxEditor(UI::Root* root = nullptr) : UI::Table{root}{
			defaultCellLayout.fillParentX().wrapY().setMargin(5);
			setCellAlignMode(Align::Layout::top_center);
			Table::setEmptyDrawer();

			buildBasic();

			boxes.emplace_back();
			boxes.back().staticData.boxData.setSize(30, 60);
			boxes.back().staticData.localTrans.rot = 30;
			boxes.back().staticData.localTrans.vec = {100, 200};
			boxes.back().drawData = boxes.back().staticData;

			boxes.emplace_back();
			boxes.back().staticData.boxData.setSize(40, 70);
			boxes.back().staticData.localTrans.rot = 70;
			boxes.back().staticData.localTrans.vec = {100, 200};
			boxes.back().drawData = boxes.back().staticData;

			originTrans = {-100, 200, 40};

			loadBinds();
		}

		void updateBoxes(){
			for(auto& entity : boxes){
				entity.drawData = entity.staticData;
			}

			switch(curOp){
				case EditOperation::moveTrans : processMove();
					break;
				case EditOperation::scale : processScale();
					break;
				case EditOperation::rotate : processRotate();
					break;
				default : break;
			}

			quadTree.clearItemsOnly();

			for(auto& box : boxes){
				box.drawData.boxData.update(box.drawData.localTrans | originTrans);
				box.intersected = false;
				quadTree.insert(box);
			}

			for(auto& box : boxes){
				quadTree.intersectAll(box, [](const EditBox& s, EditBox& o){
					o.intersected = true;
				});
			}
		}

		void update(const Core::Tick delta) override{
			Table::update(delta);

			updateBoxes();
		}

		void drawContent() const override{
			Table::drawContent();

			using namespace Graphic;
			using Graphic::Draw::Overlay;

			screen->beginDraw(&Core::BatchGroup::overlay);

			{
				GL::UniformGuard _{Assets::Shaders::coordAxisArgs, &screen->getCamera()};
				Graphic::Mesh::meshBegin();
				Graphic::Mesh::meshEnd(true, Assets::Shaders::coordAxis);
			}

			Overlay::Line::setLineStroke(2);
			Overlay::color(UI::Pal::LIGHT_GRAY);
			Overlay::Line::lineAngle(originTrans.vec.x, originTrans.vec.y, originTrans.rot, 35);

			Overlay::color(UI::Pal::PALE_GREEN);
			Overlay::Fill::rectPoint(originTrans.vec, 4);


			for(const auto& box : boxes){
				box.draw();
				if(box.selected && (curOp == EditOperation::moveTrans || curOp == EditOperation::scale)){
					float ang = 0;

					switch(transType){
						case SpaceTransType::localSpace : ang = originTrans.rot + box.drawData.localTrans.rot;
							break;
						case SpaceTransType::originSpace : ang = originTrans.rot;
							break;
						default : break;
					}

					box.drawLocalAxis(clamp, ang);
				}
			}

			Overlay::color(UI::Pal::RED_DUSK);
			Overlay::Line::setLineStroke(5.f);
			Overlay::Line::rectOrtho(MaximumBound);

			Overlay::Line::setLineStroke(3.f);
			if(boxSelection && !hasOp()){
				const auto region = boxSelection.getRegion(screen->getCursorPosInScreen());

				Overlay::color(UI::Pal::AQUA_SKY);
				if(region.area() == 0.f){
					Overlay::Line::line(region.getSrc(), region.getEnd());
				} else{
					Overlay::Line::rectOrtho(region);
				}
			}

			screen->endDraw();
		}

		UI::CursorType getCursorType() const noexcept override{
			if(touchbility != UI::TouchbilityFlags::enabled || !screen->isInbound(getCursorPos())){
				return tooltipbuilder ? UI::CursorType::regular_tip : UI::CursorType::regular;
			} else{
				return tooltipbuilder ? UI::CursorType::clickable_tip : UI::CursorType::clickable;
			}
		}
	};
}
