module;

#include <glad/glad.h>

export module Core.Renderer;

import std;
import GL.Buffer.FrameBuffer;
// import GL.Buffer.MultiSampleFrameBuffer;
import GL;
import GL.Shader;
import Event;
import Graphic.Mask;
import Graphic.Color;
import Graphic.Resizeable;
import Graphic.PostProcessor;

import Geom.Vector2D;


namespace UI{
	struct SeperateDrawable;
}


using namespace GL;
using namespace Graphic;

export namespace Core{
	class Renderer;
}

export namespace Event{
	struct DrawEvent : Event::EventType{
		::Core::Renderer* const renderer;

		[[nodiscard]] explicit DrawEvent(::Core::Renderer* const renderer)
			: renderer(renderer){}
	};


	struct Draw_After final : DrawEvent{
		[[nodiscard]] explicit Draw_After(::Core::Renderer* const renderer)
			: DrawEvent(renderer){}
	};


	struct Draw_Post final : DrawEvent{
		[[nodiscard]] explicit Draw_Post(::Core::Renderer* const renderer)
			: DrawEvent(renderer){}
	};


	struct Draw_Prepare final : DrawEvent{
		[[nodiscard]] explicit Draw_Prepare(::Core::Renderer* const renderer)
			: DrawEvent(renderer){}
	};


	struct Draw_Overlay final : DrawEvent{
		[[nodiscard]] explicit Draw_Overlay(::Core::Renderer* const renderer)
			: DrawEvent(renderer){}
	};
}

export namespace Core{
	class Renderer : virtual public ResizeableInt{
	protected:
		std::vector<ResizeableInt*> synchronizedSizedObjects{};

		int width{200}, height{200};
		Event::EventManager drawControlHook{
				Event::indexOf<Event::Draw_After>(),
				Event::indexOf<Event::Draw_Post>(),
				Event::indexOf<Event::Draw_Prepare>(),
				Event::indexOf<Event::Draw_Overlay>()
			};

		Event::Draw_Post draw_post{this};
		Event::Draw_Prepare draw_prepare{this};
		Event::Draw_After draw_after{this};
		Event::Draw_Overlay draw_overlay{this};

	public:
		GL::FrameBuffer defaultFrameBuffer{};
		GL::FrameBuffer uiPostBuffer{};
		GL::FrameBuffer effectBuffer{};
		Graphic::Mask uiBlurMask{};


		FrameBuffer* contextFrameBuffer = nullptr;

		std::stack<FrameBuffer*> frameStack{};

		[[nodiscard]] Renderer(const int w, const int h):
			defaultFrameBuffer{w, h}, uiPostBuffer{w, h}, effectBuffer{w, h}{
			contextFrameBuffer = &defaultFrameBuffer;

			frameStack.push(contextFrameBuffer);

			Renderer::resize(w, h);
		}

		Event::EventManager& getListener(){
			return drawControlHook;
		}

		~Renderer() override = default;

		Renderer(const Renderer& other) = delete;

		Renderer& operator=(const Renderer& other) = delete;

		[[nodiscard]] int getWidth() const{
			return width;
		}

		[[nodiscard]] int getHeight() const{
			return height;
		}

		[[nodiscard]] float getDrawWidth() const{
			return static_cast<float>(width);
		}

		[[nodiscard]] float getDrawHeight() const{
			return static_cast<float>(height);
		}

		[[nodiscard]] float getCenterX() const{
			return static_cast<float>(width) * 0.5f;
		}

		[[nodiscard]] float getCenterY() const{
			return static_cast<float>(height) * 0.5f;
		}


		virtual void frameBegin(FrameBuffer* frameBuffer, bool resize, const Color& initColor, GLbitfield mask);

		virtual void frameBegin(FrameBuffer* frameBuffer){
			frameBegin(frameBuffer, false, Colors::CLEAR);
		}

		virtual void frameBegin(FrameBuffer* frameBuffer, const bool resize, const Color& initColor){
			frameBegin(frameBuffer, resize, initColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//TODO lambda support?

		virtual void frameEnd(const std::function<void(FrameBuffer*, FrameBuffer*)>& func);

		virtual void frameEnd(const PostProcessor* processor);

		virtual void frameEnd(const GL::Shader* shader);

		void frameEnd(const PostProcessor& processor){
			frameEnd(&processor);
		}

		virtual void frameEnd();

		virtual void frameEnd_Quiet();

		virtual void renderUI();

		virtual void processUISperateDraw(const UI::SeperateDrawable* drawable);

		[[nodiscard]] bool sustainSize(const unsigned int w, const unsigned int h) const{
			return w == width && h == height;
		}

		void resize(int w, int h) override;

		virtual void draw();

		[[nodiscard]] std::unique_ptr<unsigned char[]> readScreen() const{
			glBindFramebuffer(GL::FrameBuffer::READ, defaultFrameBuffer.getID());
			std::unique_ptr pixels = std::make_unique<unsigned char[]>(width * height * 4);
			glReadPixels(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height), GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());

			return pixels;
		}

		virtual void drawMain(){}

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

		[[nodiscard]] Geom::Vec2 getSize() const{
			return {getDrawWidth(), getDrawHeight()};
		}

		[[nodiscard]] Geom::Vec2 getNormalized(const Geom::Vec2 p) const{
			return (p / getSize()).sub(0.5f, 0.5f).scl(2);
		}

		[[nodiscard]] Geom::Vec2& normalize(Geom::Vec2& p) const{
			return p.div(getSize()).sub(0.5f, 0.5f).scl(2);
		}
	};
}
