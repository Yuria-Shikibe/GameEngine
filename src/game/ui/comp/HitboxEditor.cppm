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
import UI.Selection;
import UI.Palette;
import UI.Label;
import UI.Root;

import std;

import UI.Creation;
import UI.Creation.Dialog;

import Geom.Shape.RectBox;
import Geom.QuadTree;
import Geom.Analytic;

import Game.Entity.Collision;

import Graphic.Draw;
import Game.Graphic.Draw;
import Assets.Graphic;

import OS.Ctrl.Bind;
import OS.Ctrl.Operation;

import ext.bool_merge;
import ext.SnapShot;
import ext.Guard;

import OS.TextInputListener;

import Font.GlyphArrangement;

namespace Game::Scene{
	/** TODO List - Hitbox Editor
	 * clean up
	 * refernce point
	 * mirrow view/global/locl X/Y
	 * read and save
	 * reference image as background
	 *
	 * projected view (trasfrom to local)
	 *
	*/
	export class HitboxEditor : public UI::Table, public OS::TextInputListener{
		static constexpr Geom::OrthoRectFloat MaximumBound{-20000, -20000, 40000, 40000};

		inline static OS::InputBindGroup editorBinds{};

		struct EditBox : Geom::QuadTreeAdaptable<EditBox>{
			ext::SnapShot<HitBoxFragment> box{};

			Geom::Vec2 currentScl = Geom::norBaseVec2<float>;
			bool intersected{};
			bool selected{};

			void resetScale(){
				processScale(~currentScl);
				box.apply();
				currentScl = Geom::norBaseVec2<float>;
			}

			void resetRotate(){
				box.cap.trans.rot = 0;
				box.cap.box.update(box.cap.trans);
				box.resume();
			}

			void resetMove(){
				box.cap.trans.vec.setZero();
				box.cap.box.update(box.cap.trans);
				box.resume();
			}

			void processScale(Geom::Vec2 scl){
				if(scl.x > 1){
					scl.x = (scl.x - Geom::norBaseVec2<float>.x) / currentScl.x + Geom::norBaseVec2<float>.x;
				}

				if(scl.y > 1){
					scl.y = (scl.y - Geom::norBaseVec2<float>.y) / currentScl.y + Geom::norBaseVec2<float>.y;
				}
				//
				// scl.lerp(tgt, scl.length() / currentScl.length());
				box.resumeProj(&HitBoxFragment::box);

				box.snap.box.offset *= scl;
				box.snap.box.sizeVec2 *= scl;

				box.snap.box.update(box.snap.trans);
			}

			void apply(){
				currentScl *= box.snap.box.sizeVec2 / box.cap.box.sizeVec2;
				box.apply();
			}

			void processRotate(const float rot){
				box.snap.trans.rot = box.cap.trans.rot + rot;
				box.snap.box.update(box.snap.trans);
			}

			[[nodiscard]] constexpr Geom::OrthoRectFloat getBound() const noexcept{
				return box.snap.box.maxOrthoBound;
			}

			[[nodiscard]] constexpr bool roughIntersectWith(const EditBox& other) const{
				return box.snap.box.overlapRough(other.box.snap.box);
			}

			[[nodiscard]] constexpr bool exactIntersectWith(const EditBox& other) const{
				return box.snap.box.overlapExact(other.box.snap.box);
			}

			[[nodiscard]] bool containsPoint(const Geom::Vec2 point) const noexcept{
				return box.snap.box.contains(point);
			}

			void draw(const bool isFocused) const{
				if(selected && isFocused){
					Graphic::Draw::Overlay::color(UI::Pal::KEY_WORD);
				} else if(intersected){
					Graphic::Draw::Overlay::color(UI::Pal::THEME);
				} else{
					Graphic::Draw::Overlay::color(UI::Pal::LIGHT_GRAY);
				}

				Graphic::Draw::Overlay::alpha(0.5f);

				Game::Draw::hitbox<Graphic::Draw::Overlay>(box.snap.box);

				Graphic::Draw::Overlay::color(UI::Pal::GRAY, 0.5f);
				Graphic::Draw::Overlay::Line::setLineStroke(1.f);
				Graphic::Draw::Overlay::Line::rectOrtho(box.snap.box.maxOrthoBound);

				Graphic::Draw::Overlay::color(UI::Pal::KEY_WORD, 1.f);
				Graphic::Draw::Overlay::Fill::rectPoint(box.snap.box.transform.vec, 3);
			}
		};

		EditBox refImageBound{};
		GL::Texture2D refImageTexture{};

		using TreeType = Geom::QuadTree<EditBox, float, false>;

		void drawLocalAxis(const Geom::Vec2 src, const Geom::Vec2 clamp, const float ang) const{
			Graphic::Draw::Overlay::Line::setLineStroke(2.0f);
			if(!Math::zero(clamp.x)){
				Graphic::Draw::Overlay::color(Graphic::Colors::RED_DUSK);
				Graphic::Draw::Overlay::Line::lineAngleCenter(src.x,
					src.y,
					ang, 50000);
			}

			if(!Math::zero(clamp.y)){
				Graphic::Draw::Overlay::color(Graphic::Colors::ACID);
				Graphic::Draw::Overlay::Line::lineAngleCenter(src.x,
					src.y,
					ang + 90, 50000);
			}
		}

		enum class EditOperation{
			none,
			cancel,
			moveTrans,
			scale,
			rotate,
		};

		enum class FrameTransType{
			screenSpace,
			originSpace,
			localSpace
		};

		//TODO uses std::variant & visit to do limited mono...
		enum class Focus{
			boxes,
			originPoint,
			referenceImage,
			mirrowAxis,
		};

		Focus currentFocus{};

		constexpr bool isFocusOn(const Focus focus) const noexcept{ return currentFocus == focus; }

		OS::File saveFile{};

		UI::BoxSelection<float> boxSelection{};

		std::list<EditBox> boxes{};
		TreeType quadTree{MaximumBound};
		std::unordered_set<EditBox*> selected{};

		void loadFromHitbox(HitBox&& hitBox){
			boxes.clear();

			for(auto&& box : hitBox.hitBoxGroup){
				box.trans.applyInv(originTrans.cap);
				boxes.push_back(EditBox{.box = ext::SnapShot{box}});
			}

			// setOriginTrans_sustainLocal(originTrans.cap);
		}

		HitBox saveToHitbox() const {
			HitBox hitBox{};

			hitBox.trans = {};
			hitBox.hitBoxGroup.reserve(boxes.size());

			for(auto& box : boxes){
				auto& back = hitBox.hitBoxGroup.emplace_back(box.box.cap);
				back.trans |= originTrans.cap;
			}

			return hitBox;
		}

		void saveData(){
			auto json = ext::json::getJsonOf(saveToHitbox());
			ext::json::append(json, "editTrans", originTrans.cap);
			std::ofstream fs{saveFile.getPath()};
			std::print(fs, "{}", json);
		}

		void readData(){
			try{
				auto json = ext::json::parse(saveFile.readString());
				auto hitBox = ext::json::getValueFrom<HitBox>(json);
				ext::json::read(json, "editTrans", originTrans.cap, {});
				originTrans.resume();
				loadFromHitbox(std::move(hitBox));
			} catch(...){
				std::println("Load Hitbox Failed : {}", saveFile);
			}
		}

		UI::Table* sideMenu{};
		UI::Viewport* screen{};

		ext::algo::merge_policy mergePolicy = ext::algo::merge_policy::replace;
		FrameTransType transType = FrameTransType::localSpace;
		EditOperation curOp = EditOperation::none;

		ext::SnapShot<Geom::Transform> originTrans{};
		ext::SnapShot<Geom::Transform> mirrowAxis{};

		float operationAccuracy = 1.0f;
		Geom::Vec2 clamp{1, 1};

		std::shared_ptr<Font::GlyphLayout> glyphLayout = Font::obtainLayoutPtr();
		std::string specifiedMovement{};

		std::optional<float> getSpecMovement() const noexcept{
			float val;
			auto [ptr, ec] = std::from_chars(specifiedMovement.data(),
				specifiedMovement.data() + specifiedMovement.size(), val);
			if(ec == std::errc{}){
				return val;
			}
			return std::nullopt;
		}

		void setTextFocused(){
			root->setTextFocus(this);
		}

		void setTextUnfocused() const{
			root->setTextFocus(nullptr);
		}

		bool isTextFocused() const{
			return root && root->textInputListener == this;
		}

		void updateGlyphLayout(){
			Font::defGlyphParser->parseWith(glyphLayout, specifiedMovement, screen->getValidWidth() * 0.75f);
		}

	public:
		void informTextInput(const unsigned codepoint, int mods) override{
			static constexpr std::string_view AcceptedChar{"1234567890,'.eEfF-+"};
			if(std::isgreater(codepoint, std::numeric_limits<char>::max()) || !AcceptedChar.contains(
				   static_cast<char>(codepoint)))
				return;

			specifiedMovement.push_back(static_cast<char>(codepoint));
			updateGlyphLayout();
		}

		void informBackSpace(int mods) override{
			if(!specifiedMovement.empty()){
				specifiedMovement.pop_back();
				updateGlyphLayout();
			}
		}

		void informDelete(int mods) override{
			informBackSpace(mods);
		}

	private:
		void setOriginTrans_sustainLocal(const Geom::Transform transform){
			for(auto& box : boxes){
				box.box.cap.trans |= originTrans.cap;
				box.box.cap.trans.applyInv(transform);
				// box.box.cap.box.update(box.box.cap.trans);
				// box.box.apply();
			}

			originTrans = transform;
		}

		void addSelection(EditBox* entity){
			if(currentFocus != Focus::boxes) return;
			selected.insert(entity);
			entity->selected = true;
		}

		void eraseSelection(EditBox* entity){
			selected.erase(entity);
			entity->selected = false;
		}

		void selectAll(){
			if(currentFocus != Focus::boxes) return;
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

		void mirrowSelected(){
			std::vector<EditBox> copy{};
			copy.reserve(boxes.size());

			for(const auto& box : selected){
				EditBox& mirrowedCopy = copy.emplace_back(*box);

				mirrowedCopy.selected = false;
				mirrowedCopy.box.cap.trans = mirrowedCopy.box.snap.trans = Geom::mirrowBy(
					                             box->box.cap.trans | originTrans.cap, mirrowAxis.cap).applyInv(
					                             originTrans.cap);
			}

			boxes.append_range(std::move(copy));
		}

		void buildBasic(){
			using namespace UI;
			sideMenu =
				&add<Table>([this](Table& sideMenu){
					 sideMenu.setFillparent();
					 sideMenu.setCellAlignMode(Align::Layout::top_center);

					 sideMenu.add<UI::Table>([this](UI::Table& transTable){
						 transTable.setEmptyDrawer();
						 transTable.defaultCellLayout.setMargin(2.f).fillParent();

						 using namespace std::string_view_literals;

						 {
							 const std::array modes{
									 std::tuple{
										 FrameTransType::screenSpace, Icons::blender_icon_orientation_view,
										 "view-space"sv
									 },
									 std::tuple{
										 FrameTransType::localSpace, Icons::blender_icon_orientation_local,
										 "local-space"sv
									 },
									 std::tuple{
										 FrameTransType::originSpace, Icons::blender_icon_orientation_global,
										 "global-space"sv
									 },
								 };

							 for(const auto& [idx, data] : modes | std::views::enumerate){
								 auto& [mode, icon, name] = data;
								 transTable.transferElem(Create::imageButton(Styles::drawer_elem_s1_noEdge.get(),
									 icon, [this, mode]{
										 transType = mode;
									 }, [this, mode, name](Button& b){
										 b.setActivatedChecker(
											 Create::ValueChecker<&HitboxEditor::transType>{this, mode});

										 b.setTooltipBuilder({
												 .followTarget = TooltipFollowTarget::cursor,
												 .builder = [name](UI::Table& hint){
													 hint.setDrawer(UI::Styles::drawer_elem_s1_noEdge.get());
													 hint.add<UI::Label>([name](UI::Label& l){
														 l.setText(l.getBundleEntry(name));
														 l.setTextScl(0.75f);
														 l.setWrap();
														 l.setEmptyDrawer();
													 }).expand();
													 hint.layout();
												 }
											 });
									 }
								 ).first);

								 if(idx & 1){
									 transTable.endline();
								 }
							 }
						 }
					 }).fillParentX().setYRatio(1.f).endLine();

					 sideMenu.endline().emplace(Create::LineCreater{}).setHeight(4.f).setPad(
						 {.bottom = 4.f, .top = 4.f}).fillParentX().endLine();

					 sideMenu.add<UI::Table>([this](UI::Table& transTable){
						 transTable.setEmptyDrawer();
						 transTable.defaultCellLayout.setMargin(2.f).fillParent();

						 using namespace std::string_view_literals;

						 {
							 const std::array modes{
									 std::tuple{
										 Focus::referenceImage, Icons::blender_icon_image_data, "reference"sv
									 },
									 std::tuple{
										 Focus::mirrowAxis, Icons::blender_icon_mod_mirror, "mirrow"sv
									 },
									 std::tuple{
										 Focus::boxes, Icons::blender_icon_pivot_boundbox,
										 "boxes"sv
									 },
									 std::tuple{
										 Focus::originPoint, Icons::blender_icon_pivot_cursor,
										 "origin"sv
									 },
								 };

							 for(const auto& [idx, data] : modes | std::views::enumerate){
								 auto& [mode, icon, name] = data;
								 transTable.transferElem(Create::imageButton(Styles::drawer_elem_s1_noEdge.get(),
									 icon, [this, mode]{
										 currentFocus = mode;
									 }, [this, mode, name](Button& b){
										 b.setActivatedChecker(
											 Create::ValueChecker<&HitboxEditor::currentFocus>{this, mode});

										 b.setTooltipBuilder({
												 .followTarget = TooltipFollowTarget::cursor,
												 .builder = [name](UI::Table& hint){
													 hint.setDrawer(UI::Styles::drawer_elem_s1_noEdge.get());
													 hint.add<UI::Label>([name](UI::Label& l){
														 l.setText(l.getBundleEntry(name));
														 l.setTextScl(0.75f);
														 l.setWrap();
														 l.setEmptyDrawer();
													 }).expand();
													 hint.layout();
												 }
											 });
									 }
								 ).first);

								 if(idx & 1){
									 transTable.endline();
								 }
							 }
						 }
					 }).fillParentX().setYRatio(1.f).endLine();

					 sideMenu.endline().emplace(Create::LineCreater{}).setHeight(4.f).setPad(
						 {.bottom = 4.f, .top = 4.f}).fillParentX().endLine();

					 sideMenu.transferElem(Create::imageButton(Styles::drawer_elem_s1_noEdge.get(),
							 UI::Icons::inbox_upload_r,
							 [this]{
								 getRoot()->showDialog(true, Create::OutputFileDialog{
										 [this](OutputFileSelector& f){
											 f.targetSuffix = Game::HitBox::HitboxFileSuffix;
											 f.suffixFilter.insert(Game::HitBox::HitboxFileSuffix);
											 f.singleSelect = true;
											 f.confirmCallback = [this](std::unordered_set<OS::File>&& files){
												 saveFile = *files.begin();
												 saveData();
											 };
										 }
									 });
							 }).first
					 ).fillParentX().setMargin(2.f).setYRatio(.5f).endLine();

					 sideMenu.transferElem(Create::imageButton(Styles::drawer_elem_s1_noEdge.get(),
							 UI::Icons::inbox_download_r,
							 [this]{
								 getRoot()->showDialog(true, Create::BasicFileDialog{
										 [this](FileSelector& f){
											 f.suffixFilter.insert(Game::HitBox::HitboxFileSuffix);
											 f.singleSelect = true;
											 f.confirmCallback = [this](std::unordered_set<OS::File>&& files){
												 saveFile = *files.begin();
												 readData();
											 };
										 }
									 });
							 }).first
					 ).fillParentX().setMargin(2.f).setYRatio(.5f).endLine();

					 sideMenu.endline().emplace(Create::LineCreater{}).setHeight(4.f).setPad(
						 {.bottom = 4.f, .top = 4.f}).fillParentX().endLine();

					 sideMenu.transferElem(Create::imageButton(Styles::drawer_elem_s1_noEdge.get(),
							 UI::Icons::blender_icon_image_data,
							 [this]{
								 getRoot()->showDialog(true, Create::BasicFileDialog{
										 [this](FileSelector& f){
											 f.suffixFilter.insert(".png");
											 f.singleSelect = true;
											 f.confirmCallback = [this](std::unordered_set<OS::File>&& files){
												 if(files.empty()) return;

												 refImageTexture = GL::Texture2D{*files.begin()};

												 Geom::RectBox& box{refImageBound.box.cap.box};
												 box.setSize(refImageTexture.getWidth(), refImageTexture.getHeight());
												 box.offset.set(box.sizeVec2).scl(-0.5f);
												 box.update(refImageBound.box.cap.trans);

												 refImageBound.box.resume();
											 };
										 }
									 });
							 }).first
					 ).fillParentX().setMargin(2.f).setYRatio(.5f).endLine();
				 })
				 .setWidth(150.f)
				 .fillParentY()
				 .as<Table>();

			screen = &add<UI::Viewport>([](UI::Viewport& section){
				// section.setEmptyDrawer();
			}).fillParent().as<UI::Viewport>();

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

		bool clamped() const noexcept{
			return clamp.area() == 0;
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
			setTextUnfocused();
			specifiedMovement.clear();
			updateGlyphLayout();
		}

		void endOp(){
			switch(currentFocus){
				case Focus::originPoint : setOriginTrans_sustainLocal(originTrans.snap);
					break;
				case Focus::boxes :{
					std::ranges::for_each(selected, &EditBox::apply);
				}
				break;
				case Focus::referenceImage : refImageBound.apply();
					break;
				default : break;
			}

			resetOp();
		}

		void cancelOp(){
			switch(currentFocus){
				case Focus::originPoint : originTrans.resume();
					break;
				case Focus::boxes : std::ranges::for_each(selected, &ext::SnapShot<HitBoxFragment>::resume,
						&EditBox::box);
					break;
				case Focus::referenceImage : refImageBound.box.resume();
					break;
				default : break;
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
					break;
				default : if(!root->textInputListener) setTextFocused();
					break;
			}
		}

		//TODO... different origin point support
		Geom::Vec2 getBoxesAvgPoint() const noexcept{
			Geom::Vec2 avgCenter{};

			for(const auto entity : selected){
				avgCenter += entity->box.snap.trans.vec;
			}

			return (avgCenter /= selected.size()) |= originTrans.cap;
		}

		void processMove(){
			const Geom::Vec2 dst{(screen->getCursorPosInScreen() - boxSelection.src).scl(operationAccuracy)};
			float ang = -originTrans.cap.rot;

			switch(currentFocus){
				case Focus::originPoint :{
					auto dstCpy = dst;

					if(clamped()){
						switch(transType){
							case FrameTransType::localSpace : ang = originTrans.cap.rot;
								break;
							case FrameTransType::originSpace : ang = 0;
								break;
							case FrameTransType::screenSpace : ang = 0;
								break;
							default : break;
						}

						dstCpy.project(clamp.copy().rotate(ang));
					}

					originTrans.resume();
					originTrans.snap.vec += dstCpy;
					break;
				}
				case Focus::boxes :{
					for(auto* entity : selected){
						auto dstCpy = dst;

						if(clamped()){
							switch(transType){
								case FrameTransType::localSpace : ang = entity->box.snap.trans.rot;
									break;
								case FrameTransType::originSpace : ang = 0;
									break;
								case FrameTransType::screenSpace : ang = -originTrans->rot;
									break;
								default : break;
							}

							dstCpy.project(clamp.copy().rotate(ang));
						} else{
							dstCpy.rotate(ang);
						}

						entity->box.snap.trans = entity->box.cap.trans;
						entity->box.snap.trans.vec += dstCpy;

						entity->box.snap.box.update(entity->box.snap.trans);
					}

					break;
				}
				case Focus::referenceImage :{
					const auto dstCpy = dst * clamp;

					refImageBound.box.snap.trans = refImageBound.box.cap.trans;
					refImageBound.box.snap.trans.vec += dstCpy;

					refImageBound.box.snap.box.update(refImageBound.box.snap.trans);

					break;
				}
				default : break;
			}
		}

		void processScale(){
			float dst = Math::max(-15.0f,
				(screen->getCursorPosInScreen() - boxSelection.src).scl(operationAccuracy).x * 0.05f);
			if(dst < 0){
				dst = Math::curve(dst, -15.0f, 0.0f) - 1.0f;
			}

			const Geom::Vec2 scl = {
					Math::max(0.001f, 1 + dst * clamp.x), Math::max(.001f, 1 + dst * clamp.y)
				};

			switch(currentFocus){
				case Focus::boxes :{
					for(const auto& entity : selected){
						entity->processScale(scl);
					}

					break;
				}

				case Focus::referenceImage :{
					refImageBound.processScale(scl);
					break;
				}

				default : break;
			}
		}

		float getAngleMovement(const Geom::Vec2 target) const noexcept{
			if(auto spec = getSpecMovement()) return spec.value();

			const float ang1 = (boxSelection.src - target).angle();
			const float ang2 = (screen->getCursorPosInScreen() - target).angle();
			const float dst = Math::Angle::angleDstWithSign(ang1, ang2);

			return dst;
		}

		void processRotate(){
			switch(currentFocus){
				case Focus::originPoint :{
					originTrans.resumeProj(&Geom::Transform::rot);
					originTrans.snap.rot += getAngleMovement(originTrans.cap.vec) * operationAccuracy;

					break;
				}
				case Focus::boxes :{
					const float dst = getAngleMovement(getBoxesAvgPoint()) * operationAccuracy;

					for(const auto entity : selected){
						entity->processRotate(dst);
					}

					break;
				}
				case Focus::referenceImage :{
					const float dst = getAngleMovement(refImageBound.box.cap.trans.vec) * operationAccuracy;

					refImageBound.processRotate(dst);
					break;
				}
				default : break;
			}
		}

		void resetTransMove(){
			switch(currentFocus){
				case Focus::originPoint : setOriginTrans_sustainLocal({0, 0, originTrans->rot});
					break;
				case Focus::boxes :{
					for(const auto& entity : selected){
						entity->resetMove();
					}
					break;
				}
				case Focus::referenceImage :{
					refImageBound.resetMove();
					break;
				}
				default : break;
			}
		}

		void resetScale(){
			switch(currentFocus){
				case Focus::boxes :{
					for(const auto& entity : selected){
						entity->resetScale();
					}
					break;
				}
				case Focus::referenceImage :{
					refImageBound.resetScale();
					break;
				}
				default : break;
			}
		}

		void resetRotate(){
			switch(currentFocus){
				case Focus::originPoint : setOriginTrans_sustainLocal({originTrans->vec, 0});
					break;
				case Focus::boxes :{
					for(const auto& entity : selected){
						entity->resetRotate();
					}
					break;
				}
				case Focus::referenceImage :{
					refImageBound.resetRotate();
					break;
				}
				default : break;
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
			::Ctrl::Operation op_mirrow{
					"op_mirrow", OS::KeyBind(Ctrl::Key::X, Ctrl::Act::Press, Ctrl::Mode::Ctrl, [this]{
						mirrowSelected();
					})
				};

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
							currentFocus = Focus::boxes;
							boxSelection.endSelect(screen->getCursorPosInScreen(), [this](const UI::Rect rect){
								//TODO apply binary op
								clearSelected();

								quadTree.intersectRect(rect, [this](EditBox& box, const UI::Rect& r){
									if(box.box.snap.box.overlapRough(r) && box.box.snap.box.overlapExact(r)){
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

						cancelOp();
						clearSelected();

						currentFocus = Focus::boxes;

						editBox.box.cap.trans.vec = screen->getCursorPosInScreen();
						editBox.box.cap.trans.applyInv(originTrans.cap);

						Geom::RectBox& box{editBox.box.cap.box};
						box.setSize(60, 60);
						box.offset.set(box.sizeVec2).scl(-0.5f);

						editBox.box.resume();

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
						resetRotate();
					})
				};

			::Ctrl::Operation clearScale{
					"clearScale", OS::InputBind(Ctrl::Key::S, Ctrl::Act::Press, Ctrl::Mode::Alt, [this]{
						resetScale();
					})
				};

			::Ctrl::Operation clearMove{
					"clearMove", OS::InputBind(Ctrl::Key::G, Ctrl::Act::Press, Ctrl::Mode::Alt, [this]{
						resetTransMove();
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
						std::move(clearScale),
						std::move(clearMove),
						std::move(clearSelection),
						std::move(confirm),
						std::move(clampX),
						std::move(clampY),
						std::move(accurate_prs),
						std::move(accurate_rls),
						std::move(op_mirrow),
					}
				};

			basicGroup.targetGroup = &editorBinds;
			basicGroup.applyToTarget();

			loadInputBinds(editorBinds);
		}

	public:
		~HitboxEditor() override{
			if(isTextFocused()){
				setTextUnfocused();
			}
		}

		explicit HitboxEditor(UI::Root* root = nullptr) : UI::Table{root}{
			defaultCellLayout.fillParentX().wrapY().setMargin(5);
			setCellAlignMode(Align::Layout::top_center);
			Table::setEmptyDrawer();

			buildBasic();

			// boxes.emplace_back();
			// boxes.back().box.cap.box.setSize(30, 60);
			// boxes.back().box.cap.trans.rot = 30;
			// boxes.back().box.cap.trans.vec = {100, 200};
			// boxes.back().box.snap = boxes.back().box.cap;
			//
			// boxes.emplace_back();
			// boxes.back().box.cap.box.setSize(40, 70);
			// boxes.back().box.cap.trans.rot = 70;
			// boxes.back().box.cap.trans.vec = {100, 200};
			// boxes.back().box.snap = boxes.back().box.cap;

			{
				refImageBound.box.cap.trans.vec = {};

				Geom::RectBox& box{refImageBound.box.cap.box};
				box.setSize(60, 60);
				box.offset.set(box.sizeVec2).scl(-0.5f);
				box.update(refImageBound.box.cap.trans);

				refImageBound.box.resume();
			}


			// originTrans = {-100, 200, 40};

			glyphLayout->setSCale(0.65f);

			loadBinds();
		}

		void updateBoxes(){
			for(auto& entity : boxes){
				entity.box.snap = entity.box.cap;
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
				box.box.snap.box.update(box.box.snap.trans | originTrans.cap);
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

			editorBinds.setActivated(!root->hasDialog());

			updateBoxes();
		}

		void drawContent() const override{
			Table::drawContent();

			using namespace Graphic;
			using Graphic::Draw::Overlay;

			screen->beginDraw(&Core::BatchGroup::overlay);

			GL::Blendings::Normal.apply();

			{
				GL::UniformGuard _{Assets::Shaders::coordAxisArgs, &screen->getCamera(), UI::Pal::BLACK};
				Graphic::Mesh::meshBegin();
				Graphic::Mesh::meshEnd(true, Assets::Shaders::coordAxis);
			}

			if(refImageTexture.valid()){
				GL::TextureRegion wrap{&refImageTexture};
				ext::GuardRef<const GL::TextureRegion*> _{Overlay::contextTexture, &wrap};

				Overlay::color(UI::Pal::GRAY, .65f);
				Overlay::mixColor(UI::Pal::GRAY, .1f);

				Game::Draw::hitbox<Overlay>(refImageBound.box.snap.box);

				Overlay::mixColor();
			}

			switch(currentFocus){
				case Focus::boxes :{
					for(const auto& box : boxes){
						if(hasOp()){
							if(clamped() && box.selected && (
								   curOp == EditOperation::moveTrans || curOp == EditOperation::scale)){
								float ang = 0;

								switch(transType){
									case FrameTransType::localSpace : ang = originTrans->rot + box.box.snap.trans.rot;
										break;
									case FrameTransType::originSpace : ang = originTrans->rot;
										break;
									default : break;
								}

								drawLocalAxis(box.box.snap.box.transform.vec, clamp, ang);
							}

							constexpr auto srcColor = UI::Pal::GRAY.copy().setA(0.3f);
							constexpr auto dstColor = UI::Pal::THEME.copy().setA(1.f);
							Overlay::Line::line(boxSelection.src, screen->getCursorPosInScreen(), srcColor, dstColor);
							Overlay::color(srcColor);
							Overlay::Fill::rectPoint(boxSelection.src, 4);
							Overlay::color(dstColor);
							Overlay::Fill::rectPoint(screen->getCursorPosInScreen(), 6);

							const auto center = getBoxesAvgPoint();
							Overlay::Line::line(center, screen->getCursorPosInScreen(), UI::Pal::PALE_GREEN,
								UI::Pal::THEME);
							Overlay::color(UI::Pal::PALE_GREEN);
							Overlay::Fill::rectPoint(center, 6);
						}
					}
					break;
				}

				case Focus::originPoint :{
					if(curOp == EditOperation::moveTrans && clamped()){
						float ang = 0;

						switch(transType){
							case FrameTransType::localSpace : ang = originTrans->rot;
								break;
							default : ang = 0;
						}

						drawLocalAxis(originTrans->vec, clamp, ang);
					} else if(curOp == EditOperation::rotate){
						Overlay::color(UI::Pal::RED_DUSK, 0.45f);
						Overlay::Line::lineAngle(originTrans.snap.vec.x, originTrans.snap.vec.y, originTrans.snap.rot,
							30000);
					}
					break;
				}

				case Focus::referenceImage :{
					Overlay::color(UI::Pal::PALE_GREEN, 0.25f);
					Game::Draw::hitbox<Graphic::Draw::Overlay>(refImageBound.box.snap.box);

					if(clamped()) drawLocalAxis(refImageBound.box.snap.box.transform.vec, clamp, 0);
					break;
				}

				default : break;
			}

			for(const auto& editBox : boxes){
				editBox.draw(currentFocus == Focus::boxes);
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

			{
				const std::array originDrawPair{
						std::pair{&originTrans.cap, 0.5f},
						std::pair{&originTrans.snap, 1.5f},
					};

				Overlay::Line::setLineStroke(3);
				for(auto& [trans, alpha] : originDrawPair){
					Overlay::color(UI::Pal::KEY_WORD, alpha);
					Overlay::Line::lineAngle(trans->vec.x, trans->vec.y, trans->rot, 80);

					Overlay::color(UI::Pal::GRAY, alpha);
					Overlay::Fill::rectPoint(trans->vec, 6);
				}
			}

			screen->draw_OrthoPort();

			glyphLayout->offset = {32, screen->getBound().getHeight() - 32};
			//screen->getValidBound().vert_01() + Geom::Vec2{4, -4};
			glyphLayout->setAlign(Align::Layout::top_left);

			glyphLayout->render();

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
