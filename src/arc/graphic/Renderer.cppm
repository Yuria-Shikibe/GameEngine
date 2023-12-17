module ;

export module Core.Renderer;

import <glad/glad.h>;
import <memory>;
import <vector>;
import <stack>;
import GL.Buffer.FrameBuffer;
import GL.GL_Exception;
import GL;
import Event;
import Graphic.Color;
import Graphic.Resizeable;
import Graphic.PostProcessor;

using namespace GL;
using namespace Graphic;

export namespace Core{
	inline Event::Draw_Post draw_post;
	inline Event::Draw_Prepare draw_prepare;
	inline Event::Draw_After draw_after;

class Renderer : virtual public ResizeableUInt {
	protected:
		std::vector<ResizeableUInt*> synchronizedSizedObjects{};
	unsigned int width{4}, height{4};

	public:
		std::unique_ptr<FrameBuffer> defaultFrameBuffer = nullptr;
		FrameBuffer* contextFrameBuffer = nullptr;

		std::stack<FrameBuffer*> frameStack{};

		[[nodiscard]] Renderer(const unsigned int w, const unsigned int h): width(w), height(h){
			defaultFrameBuffer = std::make_unique<FrameBuffer>(w, h);

			contextFrameBuffer = defaultFrameBuffer.get();

			frameStack.push(contextFrameBuffer);
		}

		~Renderer() override = default;

		Renderer(const Renderer& other) = delete;

		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;

		Renderer& operator=(Renderer&& other) = delete;

		[[nodiscard]] unsigned int getWidth() const {
			return width;
		}

		[[nodiscard]] unsigned int getHeight() const {
			return height;
		}

		virtual void frameBegin(FrameBuffer* frameBuffer, bool resize, const Color& initColor, GLbitfield mask) = 0;

		virtual void frameBegin(FrameBuffer* frameBuffer){
			frameBegin(frameBuffer, false, Colors::CLEAR);
		}

		virtual void frameBegin(FrameBuffer* frameBuffer, const bool resize, const Color& initColor){
			frameBegin(frameBuffer, resize, initColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//TODO lambda support?

		virtual void frameEnd(PostProcessor*) = 0;

		virtual void frameEnd() = 0;

		virtual void renderUI() const {
		}

		[[nodiscard]] bool sustainSize(const unsigned int w, const unsigned int h) const {
			return w == width && h == height;
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if (sustainSize(w, h))return;

			width = w;
			height = h;

			defaultFrameBuffer->resize(w, h);
			contextFrameBuffer->resize(w, h);

			GL::viewport(w, h);

			for(const auto & resizeable : synchronizedSizedObjects){
				resizeable->resize(w, h);
			}
		}

		virtual void draw(){

			defaultFrameBuffer->bind();
			GL::viewport(width, height);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			Event::generalUpdateEvents.fire(draw_prepare);

			drawMain();

			Event::generalUpdateEvents.fire(draw_post);

			Event::generalUpdateEvents.fire(draw_after);

			renderUI();

			glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFrameBuffer->getID());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		[[nodiscard]] unsigned char* readScreen() const {
			glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer->getID());
			auto* pixels = new unsigned char[width * height * 4]{0};
			glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			return pixels;
		}

		virtual void drawMain(){

		}

		virtual void postToScreen(){
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		virtual FrameBuffer* frameBufferFallback(){
			FrameBuffer* frame = frameStack.top();
			frameStack.pop();

			return frame;
		}

		virtual void registerSynchronizedResizableObject(Resizeable* obj){
			synchronizedSizedObjects.push_back(obj);
		}

		virtual void removeSizeSynchronizedResizableObject(Resizeable* obj){
			std::erase(synchronizedSizedObjects, obj);
		}

	};
}


