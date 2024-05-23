module Game.Scene.MainMenu;

import UI.InputArea;
import UI.Root;
import SideTemp;
import UI.Screen;
import Core;
import Graphic.Draw;
import Assets.Graphic;
import OS;

void func(UI::Table& table){
	table.setEmptyDrawer();

	table.add<UI::Label>([](UI::Label& label){
		label.setText("2024-5-13");
		label.getGlyphLayout()->setSCale(0.3f);
		label.setEmptyDrawer();
		label.setFillparentX();
		label.setTextAlign(Align::Mode::bottom_right);
		label.setWrap(false);
	}).wrapY().setAlign(Align::Mode::bottom_right).endLine();

	table.add<UI::Label>([](UI::Label& label){
		label.setText(std::format(
			"$<c#LIGHT_GRAY>{}-$<c#PALE_GREEN>{}",
			label.getBundleEntry("built-in-ver-desc"),
			label.getBundleEntry("built-in-ver")
		));
		label.getGlyphLayout()->setSCale(0.4f);
		label.setEmptyDrawer();
		label.setFillparentX();
		label.setTextAlign(Align::Mode::bottom_right);
		label.setWrap(false);
	}).wrapY().setAlign(Align::Mode::bottom_right).endLine();

	table.add<UI::Label>([](UI::Label& label){
		label.setText("$<c#LIGHT_GRAY>NUAA $<c#AQUA>162320129");
		label.getGlyphLayout()->setSCale(0.4f);
		label.setEmptyDrawer();
		label.setFillparentX();
		label.setTextAlign(Align::Mode::bottom_right);
		label.setWrap(false);
	}).wrapY().setAlign(Align::Mode::bottom_right).endLine();
}

void Game::Scenes::MainMenu::build(){
	setLayoutByRelative(false);
	setBorderZero();
	setEmptyDrawer();

	add<UI::Table>([](UI::Table& table){
			table.add<UI::ScrollPane>([](UI::ScrollPane& pane){
				pane.setItem<UI::Table>([](UI::Table& menu){
					menu.add<UI::Label>([](UI::Label& label){
						label.setTextAlign(Align::Mode::center);
						label.setWrap(false);
						label.setEmptyDrawer();

						label.setText(label.getBundleEntry("main-test"));
					}).expandY().setPad({.bottom = 150}).endLine();

					std::vector<std::pair<std::string_view, std::function<void(UI::Button&, bool)>>>
						tempTestBuilders{
								{"game", [](const UI::Button& b, bool){
									UI::Root* root = b.getRoot();
									root->switchScene(UI::In_Game);

									::Test::genRandomEntities();
								}},
								{"ui-test", [](const UI::Button& b, bool){
										UI::Root* root = b.getRoot();
										root->switchScene("Test");
									}},
								{"about", [](const UI::Button& b, bool){
									UI::Root* root = b.getRoot();
									root->showDialog(true, [](UI::Dialog& dialog){
										dialog.content.setLayoutByRelative(false);
										dialog.content.setEmptyDrawer();
										dialog.content.add<UI::Table>([](UI::Table& t){
											t.add<UI::InputArea>([](UI::InputArea& label){
												label.setText("Src: https://github.com/Yuria-Shikibe/GameEngine\n");
												label.setEmptyDrawer();
												label.getGlyphLayout()->setSCale(0.45f);
												label.setFillparentX();
												label.setWrap();
												label.setBorder(4.0f);
											}).setMargin(4.0f).wrapY().endLine();
											t.add<UI::Table>(func);
										}).setSizeScale(0.4f, 0.2f).setAlign(Align::Mode::center);
									});
								}},
								{"ctrl-settings", [](const UI::Button& b, bool){
									// b.getRoot()->showDialog<UI::CtrlBindDialog>();

									UI::Root* root = b.getRoot();
									root->showDialog<UI::CtrlBindDialog>();
								}},
							};

					for(const auto& [name, func] : tempTestBuilders){
						menu.add<UI::Button>([&name, &func](UI::Button& button){
							button.add<UI::Label>([&name](UI::Label& label){
								label.setTextAlign(Align::Mode::center);
								label.setWrap(false, false);
								label.setEmptyDrawer();
								label.setTextScl(0.75f);

								label.setText(label.getBundleEntry(name));
							}).setHeight(50);
							button.setCall(func);
						}).wrapY().setMargin(5.0f).setPad({.bottom = 10}).endLine();
					}

					menu.setEmptyDrawer();
				});

				pane.setEmptyDrawer();
			});
		})
		.setAlign(Align::left).setSizeScale(0.2f, 1.0f)
		.setMargin(0, 10, 0, 10).setSrcScale(0.075f, 0.0f);

	screen = &add<UI::Screen>([](UI::Screen& table){

		})
		.setAlign(Align::Mode::center_left).setSizeScale(0.3f, 0.5f)
		.setMargin(0, 10, 0, 10).setSrcScale(0.275f, 0.0f).as<UI::Screen>();

	add<UI::Table>(func)
		.setAlign(Align::Mode::bottom_right)
		.setSizeScale(0.7f, 0.15f)
		.setMargin(0, 12, 0, 12);
}

void Game::Scenes::MainMenu::drawContent() const{
	namespace Draw = Graphic::Draw;
	Scene::drawContent();

	screen->beginDraw(&Core::BatchGroup::overlay);

	GL::UniformGuard _{Assets::Shaders::coordAxisArgs, &screen->getCamera()};
	Graphic::Mesh::meshBegin(Assets::Meshes::coords);
	Graphic::Mesh::meshEnd(true);

	Draw::color(Graphic::Colors::WHITE);
	Draw::rectOrtho(Draw::globalState.contextTexture, 0, 0, 100, 100);

	screen->endDraw();
}
