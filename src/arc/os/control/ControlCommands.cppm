module;

#include <GLFW/glfw3.h>

export module OS.Ctrl.ControlCommands;

import OS.Ctrl.Bind.Constants;
import GL.Constants;

import Core.Camera;
import Core.Renderer;
import UI.Root;

import Core.Input;
import OS;
import OS.Ctrl.Bind;
import Core;
import std;

using namespace Core;

static bool screenShotRequest = false;
static float baseMoveSpeed = 60;
//TODO finish this shit
export namespace Ctrl{
	inline float disableMoveScl = 1.0f;

	void registerUIBinds(Input& input){
		for(int i = 0; i < Mouse::Count; ++i){
			//TODO auto mode resgister
			for(int mode = 0; mode < Mode::Frequent_Bound; ++mode){
				input.binds.registerBind(i, Act::Press, mode, [i, mode]{
					Core::uiRoot->onPress(i, mode);
				});

				input.binds.registerBind(i, Act::Release, mode, [i, mode]{
					Core::uiRoot->onRelease(i, mode);
				});

				// input.binds.registerMouseBind(i, Act::DoubleClick, mode, [i, mode]{
				// 	Core::uiRoot->onDoubleClick(i, mode);
				// });
			}
		}


		input.binds.registerBind({{Ctrl::Key::Esc, Act::Press}, {Ctrl::Key::Esc, Act::Repeat}}, []{
			if(Core::uiRoot->onEsc()){
				//TODO...
			}
		});

		input.velocityListeners.emplace_back([](const float x, const float y){
			Core::uiRoot->cursorVel.set(x, y);
		});


		input.scrollListeners.emplace_back([](const float x, const float y) -> void{
			if(Core::uiRoot->focusScroll()){
				Core::uiRoot->mouseScroll.set(x, y);
				Core::uiRoot->onScroll();
			}else{
				camera->setTargetScale(camera->getTargetScale() + y * 0.05f);
			}
		});
	}

	void registerCommands(Input& input){
		// NOLINT(*-non-const-parameter)
		input.binds.registerBind(OS::KeyBind(Key::A, Act::Continuous, []{
			camera->move(-baseMoveSpeed * disableMoveScl * OS::delta(), 0);
		}));
		input.binds.registerBind(OS::KeyBind(Key::D, Act::Continuous, []{
			camera->move(baseMoveSpeed * disableMoveScl * OS::delta(), 0);
		}));
		input.binds.registerBind(OS::KeyBind(Key::W, Act::Continuous, []{
			camera->move(0, baseMoveSpeed * disableMoveScl * OS::delta());
		}));
		input.binds.registerBind(OS::KeyBind(Key::S, Act::Continuous, []{
			camera->move(0, -baseMoveSpeed * disableMoveScl * OS::delta());
		}));

		input.binds.registerBind(Key::Shift_Left, Mode::Ignore,
		                            []{
			                            baseMoveSpeed = 300;
		                            },
		                            []{ baseMoveSpeed = 60; }
		);

		input.binds.registerBind(OS::KeyBind(Key::M, Act::Press, []{
			if(disableMoveScl < 1.0f){
				disableMoveScl = 1.0f;
			} else{
				disableMoveScl = 0.0f;
			}
		}));

		input.binds.registerBind(Mouse::LMB, Act::DoubleClick, []{

			if(Core::uiRoot->cursorCaptured()) return;
			auto pos = Core::input.getCursorPos();
			pos.div(Core::renderer->getWidth(), Core::renderer->getHeight()).scl(2.0f).sub(1.0f, 1.0f);
			pos *= Core::camera->getScreenToWorld();

			Core::camera->setPosition(pos);
		});

		input.binds.registerBind(Key::H, Act::Press, []{
			if(Core::uiRoot->isHidden){
				Core::uiRoot->show();
			} else{
				Core::uiRoot->hide();
			}
		});

		registerUIBinds(input);

		Core::input.binds.registerBind(Key::P, Act::Press, []{
			OS::setPause(!OS::isPaused());
		});

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
