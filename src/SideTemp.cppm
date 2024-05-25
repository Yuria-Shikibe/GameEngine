module;

#include "../src/code-gen/ReflectData_Builtin.hpp"
#include "../src/arc/util/ReflectionUtil.hpp"

export module SideTemp;

import Assets.Directories;

import Game.Core;
import Game.Content.Builtin.Turrets;
import Game.Content.Builtin.SpaceCrafts;
import Game.Pool;
import Game.Entity.Collision;
import Game.Entity.Controller;
import Game.Entity.Controller.Player;
import Game.Entity.Controller.AI;

import Math.Rand;

import UI.Elem;
import UI.Root;
import UI.Table;
import UI.Label;
import UI.ScrollPane;
import UI.Drawer;
import UI.Styles;
import UI.Button;
import UI.SliderBar;
import UI.ProgressBar;
import UI.InputArea;
import UI.FileTreeSelector;

import Game.Chamber;
import Game.Chamber.FrameTrans;
import Game.Chamber.Frame;
import Game.Chamber.Util;

import Graphic.Draw;
import Graphic.Color;

export struct TestChamberFactory : Game::ChamberFactory<Game::SpaceCraft>{
	struct TestChamberData : Game::ChamberMetaDataBase{
		Vec2 targetPos{};
		float reload{};
	};


	struct TestChamberTrait : Game::ChamberTraitFallback<EntityType, TestChamberData>{
		float reloadTime{};

		void update(Game::Chamber<EntityType>* chamber, EntityType& entity, TraitDataType& data,
					const float delta) const{
			chamber->update(delta, entity);
			data.reload += delta;
			if(data.reload > reloadTime){
				data.reload = 0;
			}
		}

		void draw(const Game::Chamber<EntityType>* chamber, const EntityType& entity, const TraitDataType& data) const{
			Graphic::Draw::setZ(entity.zLayer);
			Graphic::Draw::rectOrtho<&Core::BatchGroup::world>(Graphic::Draw::globalState.defaultSolidTexture,
															   chamber->getChamberBound());
		}
	} baseTraitTest;


	template <Concepts::Derived<TestChamberTrait> Trait = TestChamberTrait>
	using ChamberType = Game::ChamberVariant<EntityType, typename Trait::TraitDataType, TestChamberTrait>;

	std::unique_ptr<Game::Chamber<EntityType>> genChamber() const override{
		return std::make_unique<ChamberType<>>(baseTraitTest);
	}
};

REFL_REGISTER_CLASS_DEF(::TestChamberFactory::ChamberType<>)

export
template <>
struct ::Core::IO::JsonSerializator<Game::ChamberFrameData<Game::SpaceCraft>> : Game::ChamberFrameData<Game::SpaceCraft>::JsonSrl{};

export
template <>
struct ::Core::IO::JsonSerializator<Game::ChamberFrame<Game::SpaceCraft>>{
	static void write(ext::json::JsonValue& jsonValue, const Game::ChamberFrame<Game::SpaceCraft>& data){
		::Core::IO::JsonSerializator<Game::ChamberFrameData<Game::SpaceCraft>>::write(jsonValue, data);
	}

	static void read(const ext::json::JsonValue& jsonValue, Game::ChamberFrame<Game::SpaceCraft>& data){
		::Core::IO::JsonSerializator<Game::ChamberFrameData<Game::SpaceCraft>>::read(jsonValue, data);
		data.reTree();
	}
};

export template<>
struct ::Core::IO::JsonSerializator<Game::ChamberTile<Game::SpaceCraft>> : Game::ChamberJsonSrl<Game::SpaceCraft>{};

export namespace Test{
	std::unique_ptr<Game::ChamberFrameTrans<Game::SpaceCraft>> chamberFrame{};
	std::unique_ptr<Game::ChamberFactory<Game::SpaceCraft>> testFactory{std::make_unique<TestChamberFactory>()};

	void loadChamberTest(){
		OS::File fi{R"(D:\projects\GameEngine\properties\resource\test.json)"};
		OS::File pixmap{R"(D:\projects\GameEngine\properties\resource\tiles.png)"};


		if constexpr(false){
			auto pixmap_ = Graphic::Pixmap{pixmap};
			::Test::chamberFrame->getChambers() = Game::ChamberUtil::genFrameFromPixmap<Game::SpaceCraft>(
				pixmap_, {-pixmap_.getWidth() / 2, -pixmap_.getHeight() / 2});

			ext::json::JsonValue jval = ext::json::getJsonOf(::Test::chamberFrame->getChambers());

			fi.writeString(std::format("{:nf0}", jval));
		} else{
			ext::json::Json json{fi.quickRead()};

			ext::json::getValueTo(::Test::chamberFrame->getChambers(), json.getData());
		}
	}

	void genRandomEntities(){
		loadChamberTest();

		Game::EntityManage::clear();
		Math::Rand rand{};
		for(int i = 0; i < 300; ++i) {


			const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
			ptr->trans.rot  = 0.0f;//rand.random(0.0f);
			Game::read(OS::File{Assets::Dir::game.subFile(R"(hitbox\macrophage.hitbox)")}, ptr->hitBox);
			ptr->trans.vec.set(rand.range(20000), rand.range(20000));
			ptr->setHealth(5000);
			ptr->initTrait(Game::Content::Builtin::test_macrophage);

			Game::EntityManage::add(ptr);
			ptr->setHealth(500);
			ptr->physicsBody.inertialMass = 500;
			ptr->chambers.setLocalTrans(ptr->trans);
			ptr->activate();
			// ptr->zLayer -= 10;


			ptr->init();
			ptr->setTurretType(Game::Content::Builtin::test_pesterSecondary);
			ptr->controller.reset(new Game::AiController{ptr.get()});

		}

		const auto ptr = Game::EntityManage::obtain<Game::SpaceCraft>();
		ptr->trans.vec.set(0, 0);
		Game::EntityManage::add(ptr);
		Game::read(OS::File{Assets::Dir::game.subFile(R"(hitbox\pester.hitbox)")}, ptr->hitBox);

		std::array posArr{Geom::Vec2{-330, 100}, Geom::Vec2{-100, 200}, Geom::Vec2{400, 130}};

		for (auto arr : posArr){
			for(const int i : {-1, 1}){
				ptr->turretEntities.push_back(Game::EntityManage::obtainUnique<Game::TurretEntity>());
				auto& t = ptr->turretEntities.back().operator*();

				t.relativePosition += arr * Geom::Vec2(1, i);
				t.init(Game::Content::Builtin::test_pulseLaser, ptr.get());
			}
		}

		ptr->vel.vec.set(0, 0);
		ptr->setHealth(20000);
		ptr->setHealthMaximum(20000);
		ptr->initTrait(Game::Content::Builtin::test_pester);
		ptr->activate();
		ptr->chambers.operator=(std::move(*chamberFrame));
		ptr->chamberTrans.vec.x = 85;
		ptr->physicsBody.inertialMass = 4000;
		chamberFrame = std::make_unique<Game::ChamberFrameTrans<Game::SpaceCraft>>();
		ptr->controller.reset(new Game::PlayerController{ptr.get()});

	}


void setupUITest_Old(){
	const auto HUD = new UI::Scene{"Test"};

	HUD->setLayoutByRelative(false);
	HUD->setBorderZero();
	HUD->setDrawer(&UI::emptyDrawer);

	UI::InputArea* inputArea{};

	HUD->add<UI::Table>([&inputArea](UI::Table& table){
		   table.add<UI::ScrollPane>([&inputArea](UI::ScrollPane& pane){
			   inputArea = &pane.setItem<UI::InputArea, false, false>([](UI::InputArea& area){
				   area.setWrap();
				   area.setMaxTextLength(3200);

				   area.getGlyphLayout()->setSCale(0.75f);
			   	area.setText("12312312\n123123123123\n123123n");
			   });
			   pane.setEmptyDrawer();
		   }).fillParent();
	   })
	   .setAlign(Align::top_right)
	   .setSizeScale(0.3f, 0.575f).setSrcScale(0.0f, 0.25f)
	   .setMargin(10, 0, 10, 0);

	HUD->add<UI::Table>([inputArea](UI::Table& label){
		   label.add<UI::ScrollPane>([inputArea](UI::ScrollPane& pane){
			   pane.setItem<UI::Label>([inputArea](UI::Label& label){
				   label.setText([inputArea](){
					   return inputArea->getText();
				   });

				   label.setEmptyDrawer();
				   label.setFillparentX();
				   label.setWrap(false);
			   });
			   pane.setEmptyDrawer();
		   });
	   })
	   .setAlign(Align::top_left).setSizeScale(0.25f, 0.2f)
	   .setMargin(0, 10, 0, 10);


	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::Table>([](UI::Table& t){
			   t.selfMaskOpacity = 0.0f;
			   t.setBorderZero();
			   t.defaultCellLayout.setMargin({.left = 2.0f, .right = 2.f});
			   for(int i = 0; i < 8; ++i){
				   t.add<UI::Button>([i](UI::Button& button){
					   button.setCall([i](UI::Button& b, bool){
						   b.buildTooltip();
					   });

					   button.setTooltipBuilder({
							   .followTarget = UI::TooltipFollowTarget::parent,
							   .minHoverTime = UI::DisableAutoTooltip,
							   .followTargetAlign = Align::Mode::bottom_left,
							   .builder = [i](UI::Table& hint){
								   // hint.setMinimumSize({600, 300});
								   hint.setCellAlignMode(Align::Mode::top_left);
								   hint.add<UI::Label>([i](UI::Label& label){
									   label.setWrap();
									   label.setText(
										   std::format("<Hint Hover Table>\nButton$<sub>$<c#PALE_GREEN>{}$<\\sub>", i));
									   label.getGlyphLayout()->setSCale(0.65f);
									   label.setEmptyDrawer();
									   label.getBorder().expand(4.0f);
								   }).expand().endLine();
								   hint.add<UI::Button>([i](UI::Button& button){
									   button.setCall([i](UI::Button&, bool){
										   Core::uiRoot->showDialog(true, [i](UI::Table& builder){
											   builder.add<UI::FileTreeSelector>([](UI::FileTreeSelector& selector){
												   selector.gotoFile(Assets::Dir::assets.getParent(), false);
											   }).fillParent().setAlign(Align::Mode::top_center);
										   });
									   });
									   button.setTooltipBuilder({
											   .followTarget = UI::TooltipFollowTarget::parent,
											   .followTargetAlign = Align::Mode::center_right,
											   .tooltipSrcAlign = Align::Mode::center_left,
											   .builder = [](UI::Table& hintInner){
												   hintInner.setCellAlignMode(Align::Mode::top_left);
												   hintInner.add<UI::Label>([](UI::Label& label){
													   label.setWrap();
													   label.setText(std::format(
														   "<Hover Table>$<sub>$<c#PALE_GREEN>{}$<\\sub>", "Nesting"));
													   label.getGlyphLayout()->setSCale(0.65f);
													   label.setEmptyDrawer();
													   label.getBorder().expand(4.0f);
												   }).expand().endLine();
											   }
										   });
								   }).setHeight(120.0f).expandY().fillParentX();
								   hint.PointCheck = 180;
							   }
						   });
				   });
			   }
		   }).fillParent();

		   table.add<UI::Table>([](UI::Table& t){
			   t.setEmptyDrawer();
			   auto& slider = t.add<UI::SliderBar>([](UI::SliderBar& s){
				   s.setClampedOnHori();
			   }).fillParent().endLine().as<UI::SliderBar>();

			   t.add<UI::ProgressBar>([&slider](UI::ProgressBar& bar){
				   bar.progressGetter = [&slider]{
					   return slider.getProgress().x;
				   };

				   bar.PointCheck = 12;

				   bar.setTooltipBuilder({
						   .followTarget = UI::TooltipFollowTarget::parent,
						   .followTargetAlign = Align::Mode::bottom_center,
						   .tooltipSrcAlign = Align::Mode::top_center,
						   .builder = [&bar](UI::Table& hint){
							   // hint.setMinimumSize({600, 300});
							   hint.setCellAlignMode(Align::Mode::top_left);
							   hint.add<UI::Label>([&bar](UI::Label& label){
								   label.setWrap();
								   label.setText([&bar]{
									   return std::format(
										   "$<c#GRAY>Progress: $<c#LIGHT_GRAY>{:.2f}$<scl#[0.75]>$<c#PALE_GREEN>%",
										   bar.getDrawProgress() * 100.0f);
								   });
								   label.getGlyphLayout()->setSCale(0.55f);
								   label.setEmptyDrawer();
								   label.getBorder().expand(2.0f);
							   }).expand().endLine();
						   }
					   });
			   });
		   }).fillParent().setPad({.left = 2.0f});
	   })
	   .setAlign(Align::Mode::top_left)
	   .setSizeScale(0.4f, 0.08f)
	   .setSrcScale(0.25f, 0.0f)
	   .setMargin(10, 0, 0, 0);


	HUD->transferElem(new UI::Table{})
	   .setAlign(Align::Mode::top_left)
	   .setSizeScale(0.1f, 0.6f)
	   .setSrcScale(0.0f, 0.2f)
	   .setMargin(0, 0, 10, 10);


	HUD->add<UI::Table>([](UI::Table& table){
		   table.add<UI::Elem>();
		   table.lineFeed();
		   table.add<UI::Elem>();
		   table.add<UI::Elem>();
	   })
	   .setAlign(Align::Mode::bottom_left)
	   .setSizeScale(0.25f, 0.2f)
	   .setMargin(0, 10, 10, 10);


	HUD->transferElem(new UI::Table{})
	   .setAlign(Align::Mode::bottom_left)
	   .setSizeScale(0.075f, 0.2f)
	   .setSrcScale(0.25f, 0.0f)
	   .setMargin(10, 0, 10, 10);


	{
		auto pane = new UI::ScrollPane{};

		pane->setItem<UI::Table>([](UI::Table& rt){
			rt.setSize(400, 900);
			rt.setFillparentX();

			rt.add<UI::ScrollPane>([](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([](UI::Table& paneT){
					paneT.setHeight(600);
					paneT.setFillparentX();

					paneT.add<UI::Elem>();
					paneT.lineFeed();
					paneT.add<UI::Elem>();
					paneT.add<UI::Elem>();
				});
			});
			// rt->add(new UI::Elem);
			rt.lineFeed();
			rt.transferElem(new UI::Elem{});
			rt.transferElem(new UI::Elem{});
		});

		HUD->transferElem(pane).setAlign(Align::Mode::top_right).setSizeScale(0.225f, 0.25f).setMargin(10, 0, 0, 10);
	}
	//
	HUD->add<UI::Table>([](UI::Table& table){
		table.add<UI::SliderBar>([](UI::SliderBar& s){
			s.setEmptyDrawer();
				   // s.setClampedOnHori();
		}).fillParent().endLine();
	})
	   .setAlign(Align::top_right)
	   .setSizeScale(0.225f - 0.185f, 0.45f).setSrcScale(0.3f, 0.25f)
	   .setMargin(10, 10, 10, 0);
	//
	HUD->add<UI::Table>([](UI::Table& table){})
	   .setAlign(Align::Mode::bottom_right)
	   .setSizeScale(0.3f, 0.15f)
	   .setMargin(10, 0, 10, 0);

	Core::uiRoot->registerScene(std::unique_ptr<UI::Scene>{HUD});
}

}
