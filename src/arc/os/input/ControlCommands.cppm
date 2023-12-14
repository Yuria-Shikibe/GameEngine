//
// Created by Matrix on 2023/12/2.
//

module;

export module Ctrl.ControlCommands;

import Ctrl.Constants;

import Core.Input;
import OS;
import OS.Key;
import OS.Mouse;
import Graphic;
import Core;
import <array>;
import <iostream>;

using namespace Core;

static bool screenShotRequest = false;
static float baseMoveSpeed = 60;
//TODO finish this shit
export namespace Ctrl{
	void registerCommands(Input* const input) { // NOLINT(*-non-const-parameter)
		input->scrollListener.emplace_back([]([[maybe_unused]] float x, const float y) -> void {
			camera->setTargetScale(camera->getTargetScale() + y * 0.05f);
		});

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
				Graphic::windowize(window, currentMonitor, lastScreenBound, title);
				renderer->resize(lastScreenBound.getWidth(), lastScreenBound.getHeight());
			}else {
				Graphic::fullScreen(window, currentMonitor, title);
				renderer->resize(mainScreenMode->width, mainScreenMode->height);
			}
		}));

		auto keys = std::array{OS::KeyBind(GLFW_KEY_LEFT_SHIFT, Act_Continuous), OS::KeyBind(GLFW_KEY_SPACE, GLFW_PRESS) };
		input->registerKeyBindMulti(keys, []() {camera->setTargetScale(1.0f);});

		input->registerMouseBind(Ctrl::LMB, Ctrl::Act_DoubleClick, [] {

			auto pos = Core::input->getMousePos();
			pos.div(Core::renderer->getWidth(), Core::renderer->getHeight()).scl(2.0f).sub(1.0f, 1.0f).scl(1.0f, -1.0f);
			pos *= Core::camera->getScreenToWorld();

			Core::camera->setPosition(pos);
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
