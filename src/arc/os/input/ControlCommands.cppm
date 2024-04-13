module;

#include <GLFW/glfw3.h>

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
import Core;
import std;

using namespace Core;

static bool screenShotRequest = false;
static float baseMoveSpeed = 60;
//TODO finish this shit
export namespace Ctrl{
	inline float disableMoveScl = 1.0f;

	void registerCommands(Input* const input) { // NOLINT(*-non-const-parameter)
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_PRESS, []{baseMoveSpeed = 200;}));
		input->registerKeyBind(new OS::KeyBind(GLFW_KEY_LEFT_SHIFT, GLFW_RELEASE, []{baseMoveSpeed = 60;}));

		input->registerKeyBind(new OS::KeyBind(KEY_A, Act_Continuous, []{
			camera->trans(-baseMoveSpeed * disableMoveScl * OS::delta(), 0);
		}));
		input->registerKeyBind(new OS::KeyBind(KEY_D, Act_Continuous, []{
			camera->trans( baseMoveSpeed * disableMoveScl * OS::delta(), 0);
		}));
		input->registerKeyBind(new OS::KeyBind(KEY_W, Act_Continuous, []{
			camera->trans(0,  baseMoveSpeed * disableMoveScl * OS::delta());
		}));
		input->registerKeyBind(new OS::KeyBind(KEY_S, Act_Continuous, [] {
			camera->trans(0, -baseMoveSpeed * disableMoveScl * OS::delta());
		}));

		input->registerKeyBind(new OS::KeyBind(KEY_M, Act_Press, [] {
			if(disableMoveScl < 1.0f){
				disableMoveScl = 1.0f;
			}else{
				disableMoveScl = 0.0f;
			}
		}));

		// input->registerKeyBind(new OS::KeyBind(GLFW_KEY_F11, GLFW_PRESS, [] {
		// 	if(currentMonitor == mainMonitor) {
		// 		Graphic::windowize(mainWindow, currentMonitor, lastScreenBound, title);
		//
		// 		glfwRestoreWindow(mainWindow);
		//
		// 		renderer->resize(lastScreenBound.getWidth(), lastScreenBound.getHeight());
		// 	}else {
		// 		Graphic::fullScreen(mainWindow, currentMonitor, title);
		// 		renderer->resize(mainScreenMode->width, mainScreenMode->height);
		// 	}
		// }));

		auto keys = std::array{OS::KeyBind(GLFW_KEY_LEFT_SHIFT, Act_Continuous), OS::KeyBind(GLFW_KEY_SPACE, GLFW_PRESS) };
		input->registerKeyBindMulti(keys, []() {camera->setTargetScaleDef();});

		input->registerMouseBind(LMB, Act_DoubleClick, [] {
			if(Core::uiRoot->cursorCaptured())return;
			auto pos = Core::input->getMousePos();
			pos.div(Core::renderer->getWidth(), Core::renderer->getHeight()).scl(2.0f).sub(1.0f, 1.0f);
			pos *= Core::camera->getScreenToWorld();

			Core::camera->setPosition(pos);
		});

		input->registerKeyBind(KEY_H, Act_Press, [] {
			if(Core::uiRoot->isHidden) {
				Core::uiRoot->enable();
			}else {
				Core::uiRoot->disable();
			}
		});

		for(int i = 0; i < MOUSE_BUTTON_COUNT; ++i) {//TODO auto mode resgister
			Core::input->registerMouseBind(i, Act_Press, [i] {
				Core::uiRoot->onPress(i);
			});

			Core::input->registerMouseBind(i, Act_Release, [i] {
				Core::uiRoot->onRelease(i);
			});

			Core::input->registerMouseBind(i, Act_DoubleClick, [i] {
				Core::uiRoot->onDoubleClick(i);
			});
		}

		input->registerKeyBind(Ctrl::KEY_ESCAPE, Act_Press, [] {
			if(Core::uiRoot->onEsc()){
				//TODO...
			}
		});


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


		Core::input->registerKeyBind(KEY_P, Act_Press, [] {
			OS::setPause(!OS::isPaused());
		});

		{//TODO pack this into a class like screen shot manager
			// Core::input->registerKeyBind(KEY_F1, Act_Press, []() mutable  {
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
