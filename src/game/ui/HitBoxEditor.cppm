export module Game.UI.HitBoxEditor;

import OS.File;
import OS.ApplicationListener;
import Game.Graphic.Draw;
import Game.Entity.RealityEntity;
import Game.Entity.Collision;

export import Game.UI.OverlayInterface;

import Ctrl.Constants;
import Assets.Graphic;
import Graphic.Color;
import Graphic.Draw;

import Geom.Rect_Orthogonal;
import Math;

import std;

export namespace Game{
	class HitboxEntity : public RealityEntity {
	public:
		HitBox tempHitbox{};
		void calculateInScreen(const Geom::OrthoRectFloat& viewport) override{
			inScreen = true;
		}

		void draw() const override{
			drawDebug();
		}

		void drawDebug() const override{
			Graphic::Draw::color();
			Game::Draw::hitbox(tempHitbox.hitBoxGroup.back().original);

			if(controller->selected)Graphic::Draw::color(Graphic::Colors::ORANGE);
			Game::Draw::hitbox(tempHitbox.hitBoxGroup.front().original);
		}

		bool ignoreCollisionTo(const Game::RealityEntity* object) const override{
			return true;
		}

		void update(const float deltaTick) override {
			updateHitbox(deltaTick);

			tempHitbox = hitBox;
		}
	};

	//TODO impl undo
	class HitBoxEditor : public Game::OverlayInterface{
		OS::File storeFile{R"(D:\projects\GameEngine\properties\resource\assets\hitbox\pester.hitbox)"};

		Game::IDType lastQuadID = 0;
		std::unordered_set<HitboxEntity*> selected{};
		std::unordered_map<Game::IDType, std::unique_ptr<HitboxEntity>> allHitboxes{};

		Geom::Vec2 selectionBegin{Geom::SNAN2};

		/**
		 * @brief World Pos
		 */
		Geom::Vec2 opBeginMousePos{};

		enum class Operation {
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

		Operation curOp = Operation::none;

		bool hasOp() const{
			return static_cast<int>(curOp) > 1;
		}

		BoxSelectFunc boxSelectFunc = BoxSelectFunc::reallocate;

		float operationAccuracy = 1.0f;
		Geom::Vec2 clamp{1, 1};

	public:
		HitBoxEditor(){
			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press,Ctrl::Mode_Shift, [this] {
				if(!activated)return;
				for (auto& val : allHitboxes | std::views::values){
					if(val->hitBox.contains(mouseWorldPos) && !selected.contains(val.get())){
						addSelection(val.get());
						break;
					}
				}
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_2, Ctrl::Act_Press,Ctrl::Mode_Shift, [this] {
				if(!activated)return;
				for (auto& val : allHitboxes | std::views::values){
					if(val->hitBox.contains(mouseWorldPos) && selected.contains(val.get())){
						eraseSelection(val.get());
						break;
					}
				}
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_A, Ctrl::Act_Press, Ctrl::Mode_Shift, [this] {
					if(!activated)return;
					auto ptr = create();
					clearSelected();
					addSelection(ptr.get());

					allHitboxes.try_emplace(ptr->getID(), std::move(ptr));
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_A, Ctrl::Act_Press, Ctrl::Mode_NO_IGNORE, [this] {
					if(!activated)return;
					selectAll();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_S, Ctrl::Act_Press, Ctrl::Mode_Ctrl, [this] {
					if(!activated)return;
					save();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_R, Ctrl::Act_Press, Ctrl::Mode_Ctrl, [this] {
					if(!activated)return;
					read();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_DELETE, Ctrl::Act_Press, [this] {
					if(!activated)return;
					deleteSelected();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press,
				Ctrl::Mode_Ctrl, [this] {
					if(!activated)return;
					if(!this->isBoxSelecting())boxSelectBegin();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Release,[this] {
					if(!activated)return;
					if(isBoxSelecting())boxSelectEnd();
			});

			Core::input->registerMouseBind(
				Ctrl::MOUSE_BUTTON_1, Ctrl::Act_Press, [this] {
					if(!activated || !hasOp())return;
					endOp();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_R, Ctrl::Act_Press, Ctrl::Mode_NO_IGNORE,[this] {
					if(!activated)return;
					switchOp(Operation::rotate);
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_S, Ctrl::Act_Press, Ctrl::Mode_NO_IGNORE,[this] {
					if(!activated)return;
					switchOp(Operation::scale);
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_G, Ctrl::Act_Press, Ctrl::Mode_NO_IGNORE, [this] {
					if(!activated)return;
					switchOp(Operation::moveTrans);
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_R, Ctrl::Act_Press, Ctrl::Mode_Alt, [this]{
					if(!activated) return;
					if(selected.size() == 1)resetRotate();
				});

			Core::input->registerKeyBind(
				Ctrl::KEY_G, Ctrl::Act_Press, Ctrl::Mode_Alt, [this]{
					if(!activated) return;
					if(selected.size() == 1)resetTransMove();
				});

			Core::input->registerKeyBind(
				Ctrl::KEY_ESCAPE, Ctrl::Act_Press, [this] {
					if(!activated)return;
					if(hasOp())switchOp(Operation::cancel);
					else this->clearSelected();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_ENTER, Ctrl::Act_Press, [this] {
					if(!activated)return;
					endOp();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_X, Ctrl::Act_Press, Ctrl::Mode_NO_IGNORE, [this] {
					if(!activated)return;
					switchX();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_Y, Ctrl::Act_Press, Ctrl::Mode_NO_IGNORE, [this] {
					if(!activated)return;
					switchY();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_X, Ctrl::Act_Press, Ctrl::Mode_Alt, [this] {
					if(!activated)return;
					flipX();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_BACKSPACE, Ctrl::Act_Press, [this] {
					if(!activated)return;
					clip();
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_LEFT_SHIFT, Ctrl::Act_Press, [this] {
					if(!activated)return;
					this->operationAccuracy = 0.075f;
			});

			Core::input->registerKeyBind(
				Ctrl::KEY_LEFT_SHIFT, Ctrl::Act_Release, [this] {
					if(!activated)return;
					this->operationAccuracy = 1.0f;
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

		bool isBoxSelecting() const{
			return !selectionBegin.isNaN();
		}

		void boxSelectBegin(){
			selectionBegin = mouseWorldPos;
		}

		void boxSelectEnd(){
			Geom::OrthoRectFloat orthoRectBox{};
			orthoRectBox.setVert(selectionBegin.x, selectionBegin.y, mouseWorldPos.x, mouseWorldPos.y);

			Geom::RectBox rectBox{};
			rectBox.setSize(orthoRectBox.getWidth(), orthoRectBox.getHeight());
			rectBox.offset.set(rectBox.sizeVec2).scl(-0.5f);

			HitBox box{};
			box.init(rectBox);
			box.updateHitbox({orthoRectBox.getCenter(), 0});

			selected.clear();
			for (auto& entity : allHitboxes | std::views::values){
				if(entity->hitBox.collideWithRough(box) && entity->hitBox.collideWithExact(box).valid()){
					addSelection(entity.get());
				}
			}

			selectionBegin = Geom::SNAN2;
		}

		void drawBeneathUI(Core::Renderer* renderer) const override{
			renderer->frameBegin(&renderer->effectBuffer);

			for (const auto& hitbox : allHitboxes | std::views::values){
				hitbox->draw();
			}

			namespace Draw = Graphic::Draw;

			if(hasOp()){
				for (const auto entity : selected){
					auto trans = entity->hitBox.hitBoxGroup.front().relaTrans;
					Draw::Line::setLineStroke(2.0f);
					if(!Math::zero(clamp.x)){
						Draw::color(Graphic::Colors::RED_DUSK);
						Draw::Line::lineAngleCenter(trans.vec.x, trans.vec.y, 0, 50000);
					}

					if(!Math::zero(clamp.y)){
						Draw::color(Graphic::Colors::FOREST);
						Draw::Line::lineAngleCenter(trans.vec.x, trans.vec.y, 90, 50000);
					}
				}
			}

			if(this->isBoxSelecting()){
				Geom::OrthoRectFloat orthoRectBox{};
				orthoRectBox.setVert(selectionBegin.x, selectionBegin.y, mouseWorldPos.x, mouseWorldPos.y);

				Draw::color(Graphic::Colors::AQUA);
				Draw::alpha(0.15f);
				Draw::rectOrtho(Draw::getDefaultTexture(), orthoRectBox);

				Draw::alpha();
				Draw::Line::rectOrtho(orthoRectBox);
			}


			renderer->frameEnd(Assets::PostProcessors::bloom.get());
		}

		void updateGlobal(const float delta) override{
			OverlayInterface::updateGlobal(delta);

			for (const auto& entity : allHitboxes | std::ranges::views::values){
				entity->update(delta);
			}

			switch(curOp){
				case Operation::moveTrans : processMove(); break;
				case Operation::scale : processScale(); break;
				case Operation::rotate : processRotate(); break;
				default: break;
			}
		}

		void switchOp(const Operation op){
			opBeginMousePos = mouseWorldPos;
			switch(op){
				case Operation::moveTrans : curOp = Operation::moveTrans; break;
				case Operation::scale : curOp = Operation::scale; break;
				case Operation::rotate : curOp = Operation::rotate; break;
				case Operation::cancel : curOp = Operation::none; cancelOp(); break;
				default: curOp = Operation::none; endOp();
			}
		}

		void processMove() const{
			const Vec2 dst = (mouseWorldPos - opBeginMousePos).scl(operationAccuracy).mul(clamp);
			for (const auto& entity : selected){
				entity->tempHitbox = entity->hitBox;

				entity->tempHitbox.hitBoxGroup.front().relaTrans = entity->hitBox.hitBoxGroup.front().relaTrans;
				entity->tempHitbox.hitBoxGroup.front().relaTrans.vec += dst;

				entity->tempHitbox.updateHitbox(entity->hitBox.trans);
			}
		}

		void processScale() const{
			float dst = Math::max(-15.0f, (mouseWorldPos - opBeginMousePos).scl(operationAccuracy).x * 0.05f);
			if(dst < 0){
				dst = Math::curve(dst, -15.0f, 0.0f) - 1.0f;
			}
			for (const auto& entity : selected){
				entity->tempHitbox = entity->hitBox;
				entity->tempHitbox.scl({1 + dst * clamp.x, 1 + dst * clamp.y});
				entity->tempHitbox.updateHitbox(entity->hitBox.trans);
			}
		}

		void processRotate() const{
			for (const auto& entity : selected){
				auto& data = entity->tempHitbox.hitBoxGroup.front();

				const float ang1 = (opBeginMousePos - data.relaTrans.vec).angle();
				const float ang2 = (mouseWorldPos - data.relaTrans.vec).angle();
				const float dst = Math::Angle::angleDstWithSign(ang1, ang2);

				entity->tempHitbox = entity->hitBox;
				data.relaTrans.rot += dst;
				entity->tempHitbox.updateHitbox(entity->hitBox.trans);
			}
		}

		void deleteSelected(){
			for (const auto entity : selected){
				allHitboxes.erase(entity->getID());
			}

			selected.clear();
		}

		void resetTransMove() const{
			for (const auto& entity : selected){
				entity->hitBox.hitBoxGroup.front().relaTrans.vec.setZero();
				entity->tempHitbox.updateHitbox(entity->hitBox.trans);
			}
		}

		void resetRotate() const{
			for (const auto& entity : selected){
				entity->hitBox.hitBoxGroup.front().relaTrans.rot = 0;
				entity->tempHitbox.updateHitbox(entity->hitBox.trans);
			}
		}

		void clearSelected(){
			for (const auto entity : selected){
				entity->controller->selected = false;
			}

			selected.clear();
		}

		void endOp(){
			for (const auto& entity : selected){
				entity->hitBox = entity->tempHitbox;
			}

			curOp = Operation::none;

			resumeX();
			resumeY();
		}

		void cancelOp(){
			for (const auto& entity : selected){
				entity->tempHitbox = entity->hitBox;
			}

			curOp = Operation::none;

			resumeX();
			resumeY();
		}

		std::unique_ptr<HitboxEntity> create(){
			auto ptr = std::make_unique<HitboxEntity>();

			ptr->setID(lastQuadID++);

			Geom::RectBox box{};
			box.setSize(60, 60);
			box.offset.set(box.sizeVec2).scl(-0.5f);
			ptr->hitBox.init(box, {mouseWorldPos, 0});

			return ptr;
		}

		void addSelection(HitboxEntity* entity){
			selected.insert(entity);
			entity->controller->selected = true;
		}

		void eraseSelection(HitboxEntity* entity){
			selected.erase(entity);
			entity->controller->selected = false;
		}

		void flipX() const{
			for (const auto& entity : selected){
				if(Math::zero(entity->hitBox.hitBoxGroup.front().relaTrans.vec.y))continue;
				Game::flipX(entity->hitBox);
				entity->tempHitbox = entity->hitBox;
			}
		}

		void clip() const{
			for (const auto& entity : selected){
				entity->hitBox.hitBoxGroup.resize(1);
				entity->tempHitbox = entity->hitBox;
			}
		}

		void selectAll(){
			for(auto& entity : this->allHitboxes | std::ranges::views::values){
				this->addSelection(entity.get());
			}
		}

		void save(){
			const OS::File file{R"(D:\projects\GameEngine\properties\resource\assets\hitbox\target.txt)"};

			std::filesystem::path path = file.readString();

			std::ofstream writer{path, std::ios::binary | std::ios::out};

			auto view = allHitboxes | std::ranges::views::values;

			const int size = std::accumulate(view.begin(), view.end(), 0, [](const int val, decltype(allHitboxes)::value_type::second_type& entity){
				return val + entity->hitBox.size();
			});

			writer.write(reinterpret_cast<const char*>(&size), sizeof(size));

			for (const auto& entity : this->allHitboxes | std::views::values){
				for (const auto& data : entity->hitBox.hitBoxGroup){
					data.write(writer);
				}
			}
		}

		void read(){
			allHitboxes.clear();
			
			const OS::File file{R"(D:\projects\GameEngine\properties\resource\assets\hitbox\target.txt)"};

			std::ifstream reader{file.readString(), std::ios::binary | std::ios::in};

			auto view = allHitboxes | std::ranges::views::values;

			int size{};

			reader.read(reinterpret_cast<char*>(&size), sizeof(size));

			for(int i = 0; i < size; ++i){
				auto ptr = create();
				ptr->hitBox.hitBoxGroup.front().read(reader);

				this->allHitboxes.try_emplace(ptr->getID(), std::move(ptr));
			}
		}
	};
}
