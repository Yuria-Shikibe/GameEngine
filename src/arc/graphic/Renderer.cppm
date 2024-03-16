export module Core.Renderer;

import <glad/glad.h>;
import <memory>;
import <vector>;
import <stack>;
import <functional>;
import GL.Buffer.FrameBuffer;
// import GL.Buffer.MultiSampleFrameBuffer;
import GL;
import Event;
import Graphic.Color;
import Graphic.Resizeable;
import Graphic.PostProcessor;

import Geom.Vector2D;


using namespace GL;
using namespace Graphic;

export namespace Core {
	class Renderer;
}

export namespace Event {
	struct DrawEvent : Event::EventType {
		::Core::Renderer* const renderer;

		[[nodiscard]] explicit DrawEvent(::Core::Renderer* const renderer)
			: renderer(renderer) {
		}
	};

	struct Draw_After final : DrawEvent {
		[[nodiscard]] explicit Draw_After(::Core::Renderer* const renderer)
			: DrawEvent(renderer) {
		}
	};

	struct Draw_Post final : DrawEvent {
		[[nodiscard]] explicit Draw_Post(::Core::Renderer* const renderer)
			: DrawEvent(renderer) {
		}
	};

	struct Draw_Prepare final : DrawEvent {
		[[nodiscard]] explicit Draw_Prepare(::Core::Renderer* const renderer)
			: DrawEvent(renderer) {
		}
	};

	struct Draw_Overlay final : DrawEvent {
		[[nodiscard]] explicit Draw_Overlay(::Core::Renderer* const renderer)
			: DrawEvent(renderer) {
		}
	};
}

export namespace Core {
	class Renderer : virtual public ResizeableUInt {
	protected:
		std::vector<ResizeableUInt*> synchronizedSizedObjects{};

		unsigned int width{ 4 }, height{ 4 };
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

		FrameBuffer* contextFrameBuffer = nullptr;

		std::stack<FrameBuffer *> frameStack{};

		[[nodiscard]] Renderer(const unsigned int w, const unsigned int h):
			defaultFrameBuffer{ w, h }, uiPostBuffer{ w, h }, effectBuffer{w, h} {
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

		[[nodiscard]] unsigned int getWidth() const {
			return width;
		}

		[[nodiscard]] unsigned int getHeight() const {
			return height;
		}

		[[nodiscard]] float getDrawWidth() const {
			return static_cast<float>(width);
		}

		[[nodiscard]] float getDrawHeight() const {
			return static_cast<float>(height);
		}

		[[nodiscard]] float getCenterX() const {
			return static_cast<float>(width) * 0.5f;
		}

		[[nodiscard]] float getCenterY() const {
			return static_cast<float>(height) * 0.5f;
		}

		virtual void frameBegin(FrameBuffer* frameBuffer, bool resize, const Color& initColor, GLbitfield mask);

		virtual void frameBegin(FrameBuffer* frameBuffer) {
			frameBegin(frameBuffer, false, Colors::CLEAR);
		}

		virtual void frameBegin(FrameBuffer* frameBuffer, const bool resize, const Color& initColor) {
			frameBegin(frameBuffer, resize, initColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		//TODO lambda support?

		virtual void frameEnd(const std::function<void(FrameBuffer*, FrameBuffer*)>& func);

		virtual void frameEnd(PostProcessor* processor);

		virtual void frameEnd();

		virtual void renderUI();

		[[nodiscard]] bool sustainSize(const unsigned int w, const unsigned int h) const {
			return w == width && h == height;
		}

		void resize(unsigned int w, unsigned int h) override;

		virtual void draw() {
			defaultFrameBuffer.bind();
			GL::viewport(width, height);

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			drawControlHook.fire(draw_prepare);

			drawMain();

			drawControlHook.fire(draw_post);

			drawControlHook.fire(draw_after);

			renderUI();

			drawControlHook.fire(draw_overlay);

			glBindFramebuffer(GL_READ_FRAMEBUFFER, defaultFrameBuffer.getID());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
			glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		[[nodiscard]] unsigned char* readScreen() const {
			glBindFramebuffer(GL_FRAMEBUFFER, defaultFrameBuffer.getID());
			auto* pixels = new unsigned char[width * height * 4]{ 0 };
			glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			return pixels;
		}

		virtual void drawMain() {
		}

		virtual void postToScreen() {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		virtual FrameBuffer* frameBufferFallback() {
			FrameBuffer* frame = frameStack.top();
			frameStack.pop();

			return frame;
		}

		virtual void registerSynchronizedResizableObject(Resizeable* obj) {
			synchronizedSizedObjects.push_back(obj);
		}

		virtual void removeSizeSynchronizedResizableObject(Resizeable* obj) {
			std::erase(synchronizedSizedObjects, obj);
		}

		[[nodiscard]] Geom::Vec2 getSize() const {
			return {getDrawWidth(), getDrawHeight()};
		}

		[[nodiscard]] Geom::Vec2 getNormalized(const Geom::Vec2 p) const {
			return (p / getSize()).sub(0.5f, 0.5f).scl(2);
		}

		[[nodiscard]] Geom::Vec2& normalize(Geom::Vec2& p) const {
			return p.div(getSize()).sub(0.5f, 0.5f).scl(2);
		}
	};
}
