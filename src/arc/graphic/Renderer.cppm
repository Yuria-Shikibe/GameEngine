module;

#include <glad/glad.h>

export module Core.Renderer;

import std;
import GL.Buffer.FrameBuffer;
import GL;
import GL.Shader;
import ext.Event;
import Graphic.Color;
import Graphic.Resizeable;
import Graphic.PostProcessor;

import Geom.Vector2D;


namespace UI{
	struct SeperateDrawable;
}

export namespace Core{
	class Renderer;
}

export namespace ext{
	struct DrawEvent : ext::EventType{
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
	class Renderer : public Graphic::ResizeableInt{
	protected:
		std::vector<Resizeable*> synchronizedSizedObjects{};

		int width{200}, height{200};
		ext::EventManager drawControlHook{
				ext::typeIndexOf<ext::Draw_After>(),
				ext::typeIndexOf<ext::Draw_Post>(),
				ext::typeIndexOf<ext::Draw_Prepare>(),
				ext::typeIndexOf<ext::Draw_Overlay>()
			};

		ext::Draw_Post draw_post{this};
		ext::Draw_Prepare draw_prepare{this};
		ext::Draw_After draw_after{this};
		ext::Draw_Overlay draw_overlay{this};

	public:
		GL::FrameBuffer defaultFrameBuffer{};
		GL::FrameBuffer uiPostBuffer{};
		GL::FrameBuffer effectBuffer{};
		GL::FrameBuffer uiBlurMask{};


		GL::FrameBuffer* contextFrameBuffer = nullptr;

		std::stack<GL::FrameBuffer*> frameStack{};

		[[nodiscard]] Renderer(const int w, const int h):
			defaultFrameBuffer{w, h, 1, false},
			uiPostBuffer{w, h, 1, true},
			effectBuffer{w, h, 1, true},
			uiBlurMask{w, h, 1, false}{
			contextFrameBuffer = &defaultFrameBuffer;

			frameStack.push(contextFrameBuffer);

			Renderer::resize(w, h);
		}

		ext::EventManager& getListener(){
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


		virtual void frameBegin(GL::FrameBuffer* frameBuffer, bool resize, const Graphic::Color& initColor, GLbitfield mask);

		virtual void frameBegin_Quiet(GL::FrameBuffer* frameBuffer, bool resize = false);

		void frameBegin(GL::FrameBuffer* frameBuffer){
			frameBegin(frameBuffer, false, Graphic::Colors::CLEAR);
		}

		void frameBegin(GL::FrameBuffer& frameBuffer){
			frameBegin(&frameBuffer, false, Graphic::Colors::CLEAR);
		}

		void frameBegin(GL::FrameBuffer* frameBuffer, const bool resize, const Graphic::Color& initColor){
			frameBegin(frameBuffer, resize, initColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//TODO lambda support?

		virtual void frameEnd(const std::function<void(GL::FrameBuffer*, GL::FrameBuffer*)>& func);

		virtual void frameEnd(const Graphic::PostProcessor* processor);

		virtual void frameEnd(const GL::ShaderSource* shader);

		void frameEnd(const Graphic::PostProcessor& processor){
			frameEnd(&processor);
		}

		virtual void frameEnd();

		virtual GL::FrameBuffer* frameEnd_Quiet();

		virtual void renderUI();

		virtual void processUISperateDraw(const UI::SeperateDrawable* drawable);

		[[nodiscard]] bool sustainSize(const int w, const int h) const{
			return w == width && h == height;
		}

		void resize(int w, int h) override;

		virtual void draw();

		virtual void drawOverlay(){
			drawControlHook.fire(draw_after);

			renderUI();

			drawControlHook.fire(draw_overlay);
		}

		void blit() const{
#if DEBUG_CHECK
			if(&defaultFrameBuffer != contextFrameBuffer){
				throw ext::RuntimeException{"All context should fallback to default buffer when blit to screen"};
			}
#endif
			glBlitNamedFramebuffer(defaultFrameBuffer.getID(), 0,
				0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST
			);
		}

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

		virtual GL::FrameBuffer* frameBufferFallback(){
			GL::FrameBuffer* frame = frameStack.top();
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

		[[nodiscard]] Geom::Vec2 getNormalized(const Geom::Vec2 p) const noexcept{
			return (p / getSize()).sub(0.5f, 0.5f).scl(2);
		}

		Geom::Vec2& normalize(Geom::Vec2& p) const noexcept{
			return p.div(getSize()).sub(0.5f, 0.5f).scl(2);
		}
	};
}
