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

namespace Game::Scene{
	export class HitboxEditor : public UI::Table{
		static constexpr Geom::OrthoRectFloat MaximumBound{-20000, -20000, 40000, 40000};

		inline static OS::InputBindGroup editorBinds{};

		struct EditBox : Geom::QuadTreeAdaptable<EditBox>{
			HitBoxFragmentData staticData{};
			HitBoxFragmentData tempData{};

			bool intersected{};
			bool selected{};

			[[nodiscard]] constexpr Geom::OrthoRectFloat getBound() const noexcept{
				return tempData.boxData.maxOrthoBound;
			}

			[[nodiscard]] constexpr bool roughIntersectWith(const EditBox& other) const{
				return tempData.boxData.overlapRough(other.tempData.boxData);
			}

			[[nodiscard]] constexpr bool exactIntersectWith(const EditBox& other) const{
				return tempData.boxData.overlapExact(other.tempData.boxData);
			}

			[[nodiscard]] bool containsPoint(const Geom::Vec2 point) const noexcept{
				return tempData.boxData.contains(point);
			}

			void draw() const{
				if(selected){
					Graphic::Draw::Overlay::color(UI::Pal::KEY_WORD);
				}else if(intersected){
					Graphic::Draw::Overlay::color(UI::Pal::THEME);
				}else{
					Graphic::Draw::Overlay::color(UI::Pal::LIGHT_GRAY);
				}

				Graphic::Draw::Overlay::alpha(0.5f);

				Game::Draw::hitbox<Graphic::Draw::Overlay>(tempData.boxData);
			}

			void drawLocalAxis(const Geom::Vec2 clamp) const{
				Graphic::Draw::Overlay::Line::setLineStroke(2.0f);
				if(!Math::zero(clamp.x)){
					Graphic::Draw::Overlay::color(Graphic::Colors::RED_DUSK);
					Graphic::Draw::Overlay::Line::lineAngleCenter(tempData.boxData.originPoint.x, tempData.boxData.originPoint.y,
						tempData.localTrans.rot, 50000);
				}

				if(!Math::zero(clamp.y)){
					Graphic::Draw::Overlay::color(Graphic::Colors::ACID);
					Graphic::Draw::Overlay::Line::lineAngleCenter(tempData.boxData.originPoint.x, tempData.boxData.originPoint.y,
						tempData.localTrans.rot + 90, 50000);
				}
			}
		};

		enum class EditOperation {
			none,
			cancel,
			moveTrans,
			scale,
			rotate,
		};

		enum class BoxSelectFunc {
			reallocate,
			intersecton,
			combine,
			sub,
		};

		UI::BoxSelection<float> boxSelection{};

		using TreeType = Geom::QuadTree<EditBox, float, false>;

		std::vector<EditBox> boxes{};
		TreeType quadTree{MaximumBound};
		std::unordered_set<EditBox*> selected{};

		UI::Table* sideMenu{};
		UI::Screen* screen{};

		EditOperation curOp = EditOperation::none;
		BoxSelectFunc boxSelectFunc = BoxSelectFunc::reallocate;

		float operationAccuracy = 1.0f;
		Geom::Vec2 clamp{1, 1};

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
			for (const auto entity : selected){
				entity->selected = false;
			}

			selected.clear();
		}

		void buildBasic(){
			sideMenu =
				&add<Table>([this](Table& sideMenu){
					 sideMenu.setFillparent();
					 sideMenu.setCellAlignMode(Align::Layout::top_center);
					 sideMenu.defaultCellLayout.setPad({.bottom = 4.f, .top = 4.f}).setSize(80.f);

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

				if(e.mode == Ctrl::Mode::None){
					switch(e.key){
						case Ctrl::Mouse::_2 :{
							quadTree.intersectPoint(screen->getCursorPosInScreen(), [&hit](EditBox& b){
								if(!hit && b.selected)hit = &b;
							});
							if(hit)eraseSelection(hit);
							break;
						}

						case Ctrl::Mouse::_1 :{
							quadTree.intersectPoint(screen->getCursorPosInScreen(), [&hit](EditBox& b){
								if(!hit && !b.selected)hit = &b;
							});
							if(hit)addSelection(hit);
							break;
						}

						default : break;
					}
				}
			});
		}

		void switchY(){
			if(clamp.x < 1){
				resumeX();
			}else{
				limitX();
				resumeY();
			}
		}

		void switchX(){
			if(clamp.y < 1){
				resumeY();
			}else{
				limitY();
				resumeX();
			}
		}

		void limitX(){clamp.x = 0;}

		void limitY(){clamp.y = 0;}

		void resumeX(){clamp.x = 1;}

		void resumeY(){clamp.y = 1;}

		void endOp(){
			for (const auto& entity : selected){
				entity->staticData = entity->tempData;
			}

			curOp = EditOperation::none;

			resumeX();
			resumeY();

			boxSelection.setInvalid();
		}

		void cancelOp(){
			for (const auto& entity : selected){
				entity->tempData = entity->staticData;
			}

			curOp = EditOperation::none;

			resumeX();
			resumeY();
		}

		void switchOp(const EditOperation op){
			boxSelection.src = screen->getCursorPosInScreen();
			switch(op){
				case EditOperation::moveTrans : curOp = EditOperation::moveTrans; break;
				case EditOperation::scale : curOp = EditOperation::scale; break;
				case EditOperation::rotate : curOp = EditOperation::rotate; break;
				case EditOperation::cancel : curOp = EditOperation::none; cancelOp(); break;
				default: curOp = EditOperation::none; endOp();
			}
		}

		void processMove() const{
			const Geom::Vec2 dst = (screen->getCursorPosInScreen() - boxSelection.src).scl(operationAccuracy).mul(clamp);
			for (auto* entity : selected){
				entity->tempData.localTrans = entity->staticData.localTrans;
				entity->tempData.localTrans.vec += dst;

				entity->tempData.boxData.update(entity->tempData.localTrans);
			}
		}

		void processScale() const{
			float dst = Math::max(-15.0f, (screen->getCursorPosInScreen() - boxSelection.src).scl(operationAccuracy).x * 0.05f);
			if(dst < 0){
				dst = Math::curve(dst, -15.0f, 0.0f) - 1.0f;
			}
			for (const auto& entity : selected){
				entity->tempData.boxData = entity->staticData.boxData;

				const Geom::Vec2 scl = {1 + dst * clamp.x, 1 + dst * clamp.y};
				entity->tempData.boxData.offset *= scl;
				entity->tempData.boxData.sizeVec2 *= scl;

				entity->tempData.boxData.update(entity->tempData.localTrans);
			}
		}

		void processRotate() const{
			for (const auto& entity : selected){
				const float ang1 = (boxSelection.src - entity->tempData.localTrans.vec).angle();
				const float ang2 = (screen->getCursorPosInScreen() - entity->tempData.localTrans.vec).angle();
				const float dst = Math::Angle::angleDstWithSign(ang1, ang2);

				entity->tempData = entity->staticData;
				entity->tempData.localTrans.rot += dst;
				entity->tempData.boxData.update(entity->staticData.localTrans);
			}
		}

		void resetTransMove() const{
			for (const auto& entity : selected){
				entity->staticData.localTrans.vec.setZero();
				entity->tempData.boxData.update(entity->tempData.localTrans);
			}
		}

		void resetRotate() const{
			for (const auto& entity : selected){
				entity->staticData.localTrans.rot = 0;
				entity->tempData.boxData.update(entity->staticData.localTrans);
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
			//
			// Core::input.binds.registerBind(
			// 	Ctrl::Mouse::_1, Ctrl::Act::Press,
			// 	Ctrl::Mode::Ctrl, [this] {
			// 		if(!activated)return;
			// 		if(!this->isBoxSelecting())boxSelectBegin();
			// });
			//
			// Core::input.binds.registerBind(
			// 	Ctrl::Mouse::_1, Ctrl::Act::Release,[this] {
			// 		if(!activated)return;
			// 		if(isBoxSelecting())boxSelectEnd();
			// });

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
								if(box.staticData.boxData.overlapRough(r) && box.staticData.boxData.overlapExact(r)){
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

					editBox.staticData.localTrans.vec = screen->getCursorPosInScreen();
					Geom::RectBox& box{editBox.staticData.boxData};
					box.setSize(60, 60);
					box.offset.set(box.sizeVec2).scl(-0.5f);

					editBox.tempData = editBox.staticData;

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
					if(selected.size() == 1)resetRotate();
				})
			};

			::Ctrl::Operation clearMove{
				"clearMove", OS::InputBind(Ctrl::Key::G, Ctrl::Act::Press, Ctrl::Mode::Alt, [this]{
					if(selected.size() == 1)resetTransMove();
				})
			};

			::Ctrl::Operation clearSelection{
				"clearSelection", OS::InputBind(Ctrl::Key::Esc, Ctrl::Act::Press, [this]{
					if(this->hasOp())this->switchOp(EditOperation::cancel);
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
					})
				, {accurate_prs.name}};

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
			boxes.back().tempData = boxes.back().staticData;

			boxes.emplace_back();
			boxes.back().staticData.boxData.setSize(40, 70);
			boxes.back().staticData.localTrans.rot = 70;
			boxes.back().staticData.localTrans.vec = {100, 200};
			boxes.back().tempData = boxes.back().staticData;

			loadBinds();
		}

		void updateBoxes(){
			for (auto& entity : boxes){
				entity.tempData = entity.staticData;
			}

			switch(curOp){
				case EditOperation::moveTrans : processMove(); break;
				case EditOperation::scale : processScale(); break;
				case EditOperation::rotate : processRotate(); break;
				default: break;
			}

			quadTree.clearItemsOnly();

			for (auto& box : boxes){
				box.tempData.boxData.update(box.tempData.localTrans);
				box.intersected = false;
				quadTree.insert(box);
			}

			for (auto& box : boxes){
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

			for (const auto & box : boxes){
				box.draw();
				if(box.selected && (curOp == EditOperation::moveTrans || curOp == EditOperation::scale))box.drawLocalAxis(clamp);
			}

			Overlay::color(UI::Pal::RED_DUSK);
			Overlay::Line::setLineStroke(5.f);
			Overlay::Line::rectOrtho(MaximumBound);

			Overlay::Line::setLineStroke(3.f);
			if(boxSelection && !hasOp()){
				const auto region = boxSelection.getRegion(screen->getCursorPosInScreen());

				Overlay::color(UI::Pal::THEME);
				if(region.area() == 0.f){
					Overlay::Line::line(region.getSrc(), region.getEnd());
				}else{
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
