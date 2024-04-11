module;

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#if defined(_WIN64) || defined(_WIN32)
#undef APIENTRY
#include <Windows.h>
#include <dwmapi.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#endif


export module Core.Platform.Current;

export import Core.Platform;
export import RuntimeException;
import Geom.Vector2D;

import std;

export namespace GLFW{
	inline void throw_GLFW_Exception(const int error_code, const char* description) {
		throw ext::IllegalArguments{"ERROR CODE: " + std::to_string(error_code) + "\n\n" + std::string(description) + "\n\n"};
	}

	void enableMSAA(const int scale = 1){
		glfwWindowHint(GLFW_SAMPLES, scale);
	}

	void initGLFW(){
		glfwSetErrorCallback(throw_GLFW_Exception);
		if(!glfwInit()) {
			throw ext::RuntimeException{"Unable to initialize GLFW!"};
		}

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		enableMSAA();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// glfwWindowHint(GLFW_SCALE_FRAMEBUFFER, GLFW_FALSE);
		//glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
		// glfwSet
	}

	const GLFWvidmode* getVideoMode(GLFWmonitor* const monitor = glfwGetPrimaryMonitor()) {
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		return mode;
	}

	void windowRefreshCallback(GLFWwindow* window){}

	void dropCallback(GLFWwindow* window, int path_count, const char* paths[]) {}

	void charCallback(GLFWwindow* window, unsigned int codepoint);

	void charModCallback(GLFWwindow* window, unsigned int codepoint, int mods);

	void scaleCallback(GLFWwindow* window, const float xScale, const float yScale){}

	void framebufferSizeCallback(GLFWwindow* window, int width, int height);

	void mouseBottomCallBack(GLFWwindow* window, int button, int action, int mods);

	void cursorPosCallback(GLFWwindow* window, double xPos, double yPos);

	void cursorEnteredCallback(GLFWwindow* window, int entered);

	void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);

	void keyCallback(GLFWwindow* window, int key, int scanCode, int action, int mods);

	void monitorCallback(GLFWmonitor* monitor, int event);

	void maximizeCallback(GLFWwindow* window, int maximized);

	void winPosCallBack(GLFWwindow* window, int xpos, int ypos);
}

namespace Platform_Windows{
	BOOL USE_DARK_MODE = true;
	bool setStyleDark(HWND handle){
		return SUCCEEDED(DwmSetWindowAttribute(handle, DWMWA_USE_IMMERSIVE_DARK_MODE, &USE_DARK_MODE, sizeof(USE_DARK_MODE)));
	}
}

export namespace Core{
#if defined(_WIN64) || defined(_WIN32)

	struct WindowGLFW : ApplicationWindow{
		[[nodiscard]] GLFWwindow* asGLFW() const{
			return as<GLFWwindow>();
		}

		[[nodiscard]] Geom::Point2 equrySize() const override{
			Geom::Point2 p{};
			glfwGetWindowSize(as<GLFWwindow>(), &p.x, &p.y);
			return p;
		}

		[[nodiscard]] Geom::Point2 equryPos() const override{
			Geom::Point2 p{};
			glfwGetWindowPos(as<GLFWwindow>(), &p.x, &p.y);
			return p;
		}

		[[nodiscard]] Geom::Point2 equryFrameBufferSize() const override{
			Geom::Point2 p{};
			glfwGetFramebufferSize(as<GLFWwindow>(), &p.x, &p.y);
			return p;
		}

		void quitFullScreen() override{
			ApplicationWindow::quitFullScreen();
			glfwSetWindowMonitor(as<GLFWwindow>(), nullptr, lastScreenBound.getSrcX(), lastScreenBound.getSrcY(), lastScreenBound.getWidth(), lastScreenBound.getHeight(), currentMonitor.refreshRate);

			if(maximumized){
				glfwMaximizeWindow(as<GLFWwindow>());
			}
		}

		void quitMaximumizedWindow() override{
			ApplicationWindow::quitMaximumizedWindow();
		}

		void setVerticalSync() override{
			glfwSwapInterval(1);
		}

		void setOpacity(const float opacity) override{
			glfwSetWindowOpacity(asGLFW(), opacity);
		}

		float getOpacity() override{
			return glfwGetWindowOpacity(asGLFW());
		}

		void setFullScreen() override{
			ApplicationWindow::setFullScreen();
			glfwSetWindowMonitor(as<GLFWwindow>(), currentMonitor.handleAs<GLFWmonitor>(), 0, 0, currentMonitor.width, currentMonitor.height, currentMonitor.refreshRate);
		}

		void setMaximumizedWindow() override{
			ApplicationWindow::setMaximumizedWindow();
			glfwMaximizeWindow(as<GLFWwindow>());
		}



		void updateMonitorData() override{
			if(implHandle){
				auto* monitor = currentMonitor.handleAs<GLFWmonitor>();
				auto* mode = GLFW::getVideoMode(monitor);
				currentMonitor.width = mode->width;
				currentMonitor.height = mode->height;
				currentMonitor.redBits = mode->redBits;
				currentMonitor.greenBits = mode->greenBits;
				currentMonitor.blueBits = mode->blueBits;
				currentMonitor.refreshRate = mode->refreshRate;
			}else{
				new (&currentMonitor) decltype(currentMonitor);
			}
		}

		void setSize(const Geom::Point2 size) const override{
			glfwSetWindowSize(asGLFW(), size.x, size.y);
		}

		void setSrc(const Geom::Point2 size) const override{
			glfwSetWindowPos(asGLFW(), size.x, size.y);
		}

		void* getNativeHandle() override{
			return glfwGetWin32Window(asGLFW());
		}

		void resetMonitor(void* handle) override{
			currentMonitor.handle = handle;
			updateMonitorData();
		}

		void initWindow(const std::string_view appName) override{
			GLFW::initGLFW();
			// glfw windowMain creation

			//TODO setting check whether to apply fullscreen at the initialization
			implHandle = glfwCreateWindow(800, 800, appName.data(), nullptr, nullptr);

			lastScreenBound.setSize(800, 800);
			resetMonitor(glfwGetPrimaryMonitor());

			// renderer = new Renderer();
			if (implHandle == nullptr){
				glfwTerminate();
				throw ext::RuntimeException("Failed to create GLFW windowMain");
			}

			glfwMakeContextCurrent(as<GLFWwindow>());

			// glad: load all OpenGL function pointers
			// ---------------------------------------
			if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
			{
				throw ext::RuntimeException("Failed to create GLFW windowMain");
			}

			runPlatformSpecial();
		}

		void setApplicationIcon(unsigned char* data, const int w, const int h, const int bpp, const int count) override{
			GLFWimage glfWimage{w, h, data};
			glfwSetWindowIcon(asGLFW(), count, &glfWimage);
		}

		void setWindowCallback() const override{
			auto* window = asGLFW();
			glfwSetFramebufferSizeCallback(window, GLFW::framebufferSizeCallback);
			glfwSetWindowSizeCallback(window, GLFW::framebufferSizeCallback);
			glfwSetCursorPosCallback(window, GLFW::cursorPosCallback);
			glfwSetDropCallback(window, GLFW::dropCallback);
			glfwSetCursorEnterCallback(window, GLFW::cursorEnteredCallback);
			glfwSetCharCallback(window, GLFW::charCallback);
			glfwSetCharModsCallback(window, GLFW::charModCallback);

			glfwSetKeyCallback(window, GLFW::keyCallback);
			glfwSetMonitorCallback(GLFW::monitorCallback);

			glfwSetScrollCallback(window, GLFW::scrollCallback);
			glfwSetWindowContentScaleCallback(window, GLFW::scaleCallback);

			glfwSetWindowMaximizeCallback(window, GLFW::maximizeCallback);
			glfwSetWindowRefreshCallback(window, GLFW::windowRefreshCallback);
			glfwSetMouseButtonCallback(window, GLFW::mouseBottomCallBack);
			glfwSetWindowPosCallback(window, GLFW::winPosCallBack);
		}

		void runPlatformSpecial() override{
			Platform_Windows::setStyleDark(static_cast<HWND>(getNativeHandle()));
		}
	};

	struct WindowsPlatformHandle : PlatformHandle{
		explicit WindowsPlatformHandle(const std::string_view applicationName)
			: PlatformHandle{applicationName}{
			window = std::make_unique<WindowGLFW>();
		}

		~WindowsPlatformHandle() override{
			glfwTerminate();
		}

		[[nodiscard]] bool shouldExit() const override{
			return glfwWindowShouldClose(window->as<GLFWwindow>());
		}

		void prepareExit() const override{
			glfwSetWindowShouldClose(window->as<GLFWwindow>(), true);
		}

		void pollEvents() const override{
			glfwSwapBuffers(window->as<GLFWwindow>());
			glfwPollEvents();
		}

		[[nodiscard]] std::string getClipboard() const override{
			if(const char* ptr = glfwGetClipboardString(window->as<GLFWwindow>())){
				std::string str{ptr};
				std::erase_if(str, [](const char c){return c == '\r';});
				return str;
			}
			return {};
		}

		void setClipboard(std::string_view text) const override{
			glfwSetClipboardString(window->as<GLFWwindow>(), text.data());
		}

		[[nodiscard]] DeltaSetter getGlobalDeltaSetter() const override{
			return getDelta;
		}

		[[nodiscard]] TimerSetter getGlobalTimeSetter() const override{
			return getTime;
		}

	private:
		static float getTime(){
			return static_cast<float>(glfwGetTime());
		}
		static float getDelta(const float last){
			return static_cast<float>(glfwGetTime()) - last;
		}


	};


	using CurrentPlatfrom = WindowsPlatformHandle;
#else
#error Not Support !
#endif

	void initCurrentPlatform(std::unique_ptr<PlatformHandle>& toInit, const std::string_view name, const int argc = 0, char* argv[] = nullptr){
		toInit.reset(new CurrentPlatfrom{name});
		toInit->initArgs(argc, argv);
	}
}

