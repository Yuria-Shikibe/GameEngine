module ;

export module Core.Renderer;

import <glad/glad.h>;
import <memory>;
import <vector>;
import <stack>;
import GL.Buffer.FrameBuffer;
import GL.GL_Exception;
import Event;
import Graphic.Color;
import Graphic.Resizeable;

using namespace GL;
using namespace Graphic;

export namespace Core{
	inline Event::Draw_Post draw_post;
	inline Event::Draw_Prepare draw_prepare;
	inline Event::Draw_After draw_after;

class Renderer : virtual public ResizeableInt {
	protected:
		std::vector<ResizeableInt*> synchronizedSizedObjects;
	int width{4}, height{4};

	public:
		std::unique_ptr<FrameBuffer> defaultFrameBuffer = nullptr;
		FrameBuffer* contextFrameBuffer = nullptr; //TODO uses stack to impl multi layer

		std::stack<FrameBuffer*> frameStack;

		Renderer(const int w, const int h): width(w), height(h){
			defaultFrameBuffer = std::make_unique<FrameBuffer>(w, h);

			contextFrameBuffer = defaultFrameBuffer.get();

			frameStack.push(contextFrameBuffer);
		}

		~Renderer() override = default;

		Renderer(const Renderer& other) = delete;

		Renderer(Renderer&& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;

		Renderer& operator=(Renderer&& other) = delete;

		[[nodiscard]] int getWidth() const {
			return width;
		}

		[[nodiscard]] int getHeight() const {
			return height;
		}

		virtual void frameBegin(FrameBuffer& frameBuffer, const bool resize = false, const Color& initColor = Colors::CLEAR, const GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) = 0;

		virtual void frameEnd() = 0;

		[[nodiscard]] bool sustainSize(const int w, const int h) const {
			return w == width && h == height;
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if (w == width && h == height)return;

			width = w;
			height = h;

			defaultFrameBuffer->resize(w, h);

			for(const auto & resizeable : synchronizedSizedObjects){
				resizeable->resize(w, h);
			}
		}

		virtual void draw(){
			defaultFrameBuffer->bind();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			Event::generalUpdateEvents.fire(draw_prepare);

			drawMain();

			Event::generalUpdateEvents.fire(draw_post);

			Event::generalUpdateEvents.fire(draw_after);

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

		virtual void registerSynchronizedObject(Resizeable* obj){
			synchronizedSizedObjects.push_back(obj);
		}

		virtual void removeSizeSynchronizedObject(Resizeable* obj){
			std::erase(synchronizedSizedObjects, obj);
		}

	};
}


