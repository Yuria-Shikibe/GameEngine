//
// Created by Matrix on 2023/12/2.
//

module;

export module Ctrl.ControlCommands;

import Ctrl.Constants;
import GL.Constants;

import Core.Camera;
import Core.Renderer;
import UI.Root;

import Core.Input;
import OS;
import OS.KeyBind;
import OS.MouseBind;
import Graphic;
import Core;
import std;
import <GLFW/glfw3.h>;

using namespace Core;

static bool screenShotRequest = false;
static float baseMoveSpeed = 60;
//TODO finish this shit
export namespace Ctrl{
	void registerCommands(Input* const input) { // NOLINT(*-non-const-parameter)
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, []{baseMoveSpeed = 200;}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE, []{baseMoveSpeed = 60;}));

		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_A, Act_Continuous, []{camera->trans(-baseMoveSpeed * OS::delta(), 0);}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_D, Act_Continuous, []{camera->trans( baseMoveSpeed * OS::delta(), 0);}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_W, Act_Continuous, []{camera->trans(0,  baseMoveSpeed * OS::delta());}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_S, Act_Continuous, [] {
			camera->trans(0, -baseMoveSpeed * OS::delta());
		}));

		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_F11, GLFW_PRESS, [] {
			if(currentMonitor == mainMonitor) {
				Graphic::windowize(mainWindow, currentMonitor, lastScreenBound, title);

				glfwRestoreWindow(mainWindow);

				renderer->resize(lastScreenBound.getWidth(), lastScreenBound.getHeight());
			}else {
				Graphic::fullScreen(mainWindow, currentMonitor, title);
				renderer->resize(mainScreenMode->width, mainScreenMode->height);
			}
		}));

		auto keys = std::array{OS::KeyBind(GLFW_KEY_LEFT_SHIFT, Act_Continuous), OS::KeyBind(GLFW_KEY_SPACE, GLFW_PRESS) };
		input->registerKeyBindMulti(keys, []() {camera->setTargetScaleDef();});

		input->registerMouseBind(Ctrl::LMB, Ctrl::Act_DoubleClick, [] {
			if(Core::uiRoot->cursorCaptured())return;
			auto pos = Core::input->getMousePos();
			pos.div(Core::renderer->getWidth(), Core::renderer->getHeight()).scl(2.0f).sub(1.0f, 1.0f);
			pos *= Core::camera->getScreenToWorld();

			Core::camera->setPosition(pos);
		});

		input->registerKeyBind(Ctrl::KEY_H, Ctrl::Act_Press, [] {
			if(Core::uiRoot->allHidden) {
				Core::uiRoot->enable();
			}else {
				Core::uiRoot->disable();
			}
		});

		for(int i = 0; i < Ctrl::MOUSE_BUTTON_COUNT; ++i) {//TODO auto mode resgister
			Core::input->registerMouseBind(i, Ctrl::Act_Press, [i] {
				Core::uiRoot->onPress(i);
			});

			Core::input->registerMouseBind(i, Ctrl::Act_Release, [i] {
				Core::uiRoot->onRelease(i);
			});

			Core::input->registerMouseBind(i, Ctrl::Act_DoubleClick, [i] {
				Core::uiRoot->onDoubleClick(i);
			});
		}


		Core::input->cursorMoveListeners.emplace_back([](const float x, const float y) {
			Core::uiRoot->cursorPos.set(x, y);
		});

		Core::input->velocityListeners.emplace_back([](const float x, const float y) {
			Core::uiRoot->cursorVel.set(x, y);
		});

		Core::input->scrollListeners.emplace_back([](const float x, const float y) {
			Core::uiRoot->mouseScroll.set(x, y);
			Core::uiRoot->onScroll();
		});

		input->scrollListeners.emplace_back([]([[maybe_unused]] float x, const float y) -> void {
			if(!Core::uiRoot->focusScroll())camera->setTargetScale(camera->getTargetScale() + y * 0.05f);
		});


		Core::input->registerKeyBind(Ctrl::KEY_P, Ctrl::Act_Press, [] {
			OS::setPause(!OS::isPaused());
		});

		{//TODO pack this into a class like screen shot manager
			// Core::input->registerKeyBind(Ctrl::KEY_F1, Ctrl::Act_Press, []() mutable  {
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
