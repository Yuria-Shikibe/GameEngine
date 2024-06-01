module;

export module OS.Ctrl.ControlCommands;

import OS.Ctrl.Bind.Constants;
import GL.Constants;

import Core.Camera;
import Core.Renderer;
import UI.Root;
import UI.Label;
import UI.Button;

import Core.Input;
import OS;
import OS.Ctrl.Bind;
import Core;
import std;

static bool screenShotRequest = false;
static float baseMoveSpeed = 60;
//TODO finish this shit
export namespace Ctrl{
	inline float disableMoveScl = 1.0f;

	//Built-in
	void registerUIBinds(Core::Input& input){
		for(int i = 0; i < Mouse::Count; ++i){
			for(int mode = 0; mode < Mode::Frequent_Bound; ++mode){
				input.binds.registerBind(i, Act::Press, mode, [i, mode]{
					Core::uiRoot->onPress(i, mode);
				});

				input.binds.registerBind(i, Act::Release, mode, [i, mode]{
					Core::uiRoot->onRelease(i, mode);
				});

				input.binds.registerBind(i, Act::DoubleClick, mode, [i, mode]{
					Core::uiRoot->onDoubleClick(i, mode);
				});
			}
		}


		input.binds.registerBind(Ctrl::Key::Esc, Act::Repeat, []{
			if(Core::uiRoot->onEsc()){
				if(auto* main = Core::uiRoot->getScenes().tryFind(UI::Menu_Main);
					!main || Core::uiRoot->currentScene == main->get()) return;
			}
		});

		input.binds.registerBind(Ctrl::Key::Esc, Act::Press, []{
			if(Core::uiRoot->onEsc()){
				if(auto* main = Core::uiRoot->getScenes().tryFind(UI::Menu_Main);
					!main || Core::uiRoot->currentScene == main->get()) return;

				//TODO Move these to other place
				Core::uiRoot->showDialog(true, [](UI::Dialog& dialog){
					dialog.content.setEmptyDrawer();
					dialog.content.setLayoutByRelative(false);
					dialog.content.add<UI::Table>([&dialog](UI::Table& inner){
						inner.setLayoutByRelative(false);
						inner.add<UI::Label>([](UI::Label& label){
							label.setText(label.getBundleEntry("back-to-main-menu"));
							label.setTextScl(0.8f);
							label.setEmptyDrawer();
							label.setTextAlign(Align::Layout::center);
						}).setSizeScale(1.0f, 0.5f).setAlign(Align::Layout::top_center).endLine();

						inner.add<UI::Table>([&dialog](UI::Table& bt){
							bt.setEmptyDrawer();
							bt.add<UI::Button>([&dialog](UI::Button& confirm){
								confirm.setFillparentY();
								confirm.add<UI::Label>([](UI::Label& label){
									label.setText(label.getBundleEntry("yes"));
									label.setTextScl(0.8f);
									label.setEmptyDrawer();
									label.setTextAlign(Align::Layout::center);
								});
								confirm.setCall([&dialog](auto&, auto){
									dialog.destroy();
									Core::uiRoot->switchScene(UI::Menu_Main);
								});
							}).setMargin({.right = 3});

							bt.add<UI::Button>([&dialog](UI::Button& confirm){
								confirm.setFillparentY();
								confirm.add<UI::Label>([](UI::Label& label){
									label.setText(label.getBundleEntry("no"));
									label.setTextScl(0.8f);
									label.setEmptyDrawer();
									label.setTextAlign(Align::Layout::center);
								});
								confirm.setCall([&dialog](auto&, auto){
									dialog.destroy();
								});
							}).setMargin({.left = 3});
						}).setSizeScale(1.0f, 0.4f).setAlign(Align::Layout::bottom_center);
					}).setSizeScale(0.5f, 0.175f).setAlign(Align::Layout::center);
				});
			}
		});

		input.velocityListeners.emplace_back([](const float x, const float y){
			Core::uiRoot->cursorVel.set(x, y);
		});


		input.scrollListeners.emplace_back([](const float x, const float y) -> void{
			if(Core::uiRoot->focusScroll()){
				Core::uiRoot->mouseScroll.set(x, y);
				Core::uiRoot->onScroll();
			} else{
				if(Core::focus.camera) Core::focus.camera->setTargetScale(
					Core::focus.camera->getTargetScale() + y * 0.05f);
			}
		});
	}

	void registerCommands(Core::Input& input){
		registerUIBinds(input);

		{
			//TODO pack this into a class like screen shot manager
			// Core::input->binds.registerKeyBind(KEY_F1, Act_Press, []() mutable  {
			// 	screenShotRequest = true;
			// });

			// Event::generalUpdateEvents.on<Event::Draw_After>([](const Event::Draw_After& a){
			// 	if(screenShotRequest) {
			// 		const Graphic::Pixmap pixmap{Core::renderer->getWidth(), Core::renderer->getHeight(), Core::renderer->defaultFrameBuffer->readPixelsRaw()};
			// 		//
			// 		auto&& f = Assets::screenshotDir.subFile("lastShot.png");
			// 		//
			// 		pixmap.write(f, true);
			//
			// 		screenShotRequest = false;
			// 	}
			// });
		}
	}
}
