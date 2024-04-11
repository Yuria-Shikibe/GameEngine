module;

#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
#include <dwmapi.h>
#elif defined(__linux__)
#include <unistd.h>
#include <limits.h>
#endif

export module Core.Platform;

import std;
import Geom.Rect_Orthogonal;
import Geom.Vector2D;
import OS.File;

export namespace Core{
	struct MonitorData{
		void* handle{nullptr};
		/*! The width, in screen coordinates, of the video mode. */
		int width{};
		/*! The height, in screen coordinates, of the video mode. */
		int height{};
		/*! The bit depth of the red channel of the video mode. */
		int redBits{};
		/*! The bit depth of the green channel of the video mode. */
		int greenBits{};
		/*! The bit depth of the blue channel of the video mode. */
		int blueBits{};
		/*! The refresh rate, in Hz, of the video mode. */
		int refreshRate{};

		template <typename T>
		T* handleAs(){
			return static_cast<T*>(handle);
		}
	};

	struct ApplicationWindow{
		virtual ~ApplicationWindow() = default;

		void* implHandle{nullptr};

		MonitorData currentMonitor{};
		Geom::OrthoRectInt lastScreenBound{};

		bool maximumized{false};
		bool fullScreen{false};

		virtual void* getNativeHandle(){return nullptr;}

		template <typename T>
		[[nodiscard]] T* as() const{
			return static_cast<T*>(implHandle);
		}

		[[nodiscard]] virtual Geom::Point2 equrySize() const {
			return {};
		}

		[[nodiscard]] virtual Geom::Point2 equryPos() const {
			return {};
		}

		[[nodiscard]] virtual Geom::Point2 equryFrameBufferSize() const {
			return {};
		}

		virtual void setSize(const Geom::Point2 size) const {}

		virtual void setSrc(const Geom::Point2 size) const {}

		virtual void resetMonitor(void* handle){

		}

		virtual void initWindow(const std::string_view appName){

		}

		virtual void updateMonitorData(){

		}

		virtual void setFullScreen(){
			fullScreen = true;
		}

		virtual void quitFullScreen(){
			fullScreen = false;
		}

		virtual void setMaximumizedWindow(){
			maximumized = true;
		}

		virtual void quitMaximumizedWindow(){
			maximumized = false;
		}

		virtual void setVerticalSync(){

		}

		virtual void setFullScreen_Windowed(){

		}

		virtual void setOpacity(float opacity){

		}

		virtual float getOpacity(){
			return 1.0f;
		}

		void informResize(const int w, const int h){
			lastScreenBound.setSize(w, h);
		}

		void informMove(const int x, const int y){
			lastScreenBound.setSrc(x, y);
		}

		[[nodiscard]] bool isSmallWindowed() const{
			return !maximumized && !fullScreen;
		}

		virtual void setApplicationIcon(unsigned char* data, int w, int h, int bpp, int count){

		}

		virtual void runPlatformSpecial(){

		}

		virtual void setWindowCallback() const = 0;
	};

	struct PlatformHandle{
		std::string applicationName{};

		explicit PlatformHandle(const std::string_view applicationName)
			: applicationName{applicationName}{}

		std::unique_ptr<ApplicationWindow> window{};

		std::vector<std::string> appArgs{};

		void initArgs(const int argc, char* argv[]){
			appArgs.reserve(argc);
			for(int i = 0; i < argc; ++i)appArgs.emplace_back(argv[i]);
		}

		virtual ~PlatformHandle() = default;

		[[nodiscard]] virtual OS::File getProcessFile() const{
			return OS::File{appArgs[0]};
		}

		[[nodiscard]] OS::File getProcessFileDir() const{
			return getProcessFile().getParent();
		}

		[[nodiscard]] virtual std::string getClipboard() const{return {};}

		virtual void setClipboard(std::string_view text) const{}

		[[nodiscard]] virtual bool openURL(std::string_view url) const {return false;}

		[[nodiscard]] virtual std::string_view getEnv(std::string_view key) const {return {};}

		[[nodiscard]] virtual bool shouldExit() const = 0;

		virtual void prepareExit() const = 0;

		virtual void pollEvents() const = 0;

		using TimerSetter = float(*)();
		using DeltaSetter = float(*)(float);

		[[nodiscard]] virtual TimerSetter getGlobalTimeSetter() const {
			return nullptr;
		}

		[[nodiscard]] virtual DeltaSetter getGlobalDeltaSetter() const {
			return nullptr;
		}
	};

	OS::File getSelf_runTime(){
		std::string pathName{};
#if defined(_WIN64) || defined(_WIN32)
		char absolutePath[MAX_PATH] = {};
		GetModuleFileNameA(nullptr, absolutePath, MAX_PATH);
		pathName = absolutePath;
#elif defined(__linux__)
		char result[PATH_MAX];
	    ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
	    pathName = std::string(result, (count > 0) ? count : 0);
#endif
		return OS::File{pathName};
	}
}
