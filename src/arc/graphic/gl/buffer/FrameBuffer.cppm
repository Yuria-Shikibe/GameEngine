module;

export module GL.Buffer.FrameBuffer;

import <glad/glad.h>;
import <memory>;

import GL;
import GL.Texture.Texture2D;
import GL.Buffer;
import GL.Buffer.RenderBuffer;
import Graphic.Resizeable;
import Graphic.Color;
import RuntimeException;
import <numeric>;
import <ranges>;
import <vector>;

export namespace GL{
	class FrameBuffer : public GLBuffer, public Graphic::ResizeableUInt
	{
	protected:
		unsigned int width = 0, height = 0;
		std::vector<std::unique_ptr<Texture2D>> samples{};

		std::unique_ptr<RenderBuffer> renderBuffer{nullptr};

	public:
		static constexpr std::array<GLenum, 32> ALL_COLOR_ATTACHMENTS {
			[] {
				std::array<GLenum, 32> arr{};

				for(int i = 0; i < 32; ++i) {
					arr[i] = GL_COLOR_ATTACHMENT0 + i;
				}

				return arr;
			}()
		};

		static constexpr GLenum DEF = GL_FRAMEBUFFER;
		static constexpr GLenum DRAW = GL_DRAW_FRAMEBUFFER;
		static constexpr GLenum READ = GL_READ_FRAMEBUFFER;

		FrameBuffer() = default;

		FrameBuffer(const unsigned int w, const unsigned int h, const int colorAttachments = 1) : width(w), height(h){
			targetFlag = GL_FRAMEBUFFER;
			glGenFramebuffers(1, &bufferID);
			FrameBuffer::bind();
			bindColorAttachments<Texture2D>(colorAttachments);
			bindRenderBuffer<RenderBuffer>();
			FrameBuffer::unbind();
		}

		template <Concepts::Derived<Texture2D> TexType, typename ...Args>
		void bindColorAttachments(const int colorAttachments = 1, Args... args) {
			samples.resize(colorAttachments);
			samples.reserve(colorAttachments);

			for(int i = 0; i < getColorAttachmentsCount(); ++i) {
				auto& sample = samples.at(i);
				sample = std::make_unique<TexType>(width, height, args...);
				sample->setScale(GL_LINEAR, GL_LINEAR);
				glFramebufferTexture2D(targetFlag, GL_COLOR_ATTACHMENT0 + i, samples.at(i)->getTargetFlag(), samples.at(i)->getID(), 0);
			}

			enableMainOnly();
		}

		template <Concepts::Derived<RenderBuffer> RBType, typename ...Args>
		void bindRenderBuffer(Args... args) {
			renderBuffer = std::make_unique<RBType>(width, height, args...);
			glFramebufferRenderbuffer(targetFlag, GL_DEPTH_STENCIL_ATTACHMENT, renderBuffer->getTargetFlag(), renderBuffer->getID());
		}

		~FrameBuffer() override{
			glDeleteFramebuffers(1, &bufferID);
			GL::cleari(bufferID);
		}

		void enableMainOnly() const {
			glDrawBuffers(1, ALL_COLOR_ATTACHMENTS.data());
		}

		void enableAll() const {
			glDrawBuffers(getColorAttachmentsCount(), ALL_COLOR_ATTACHMENTS.data());
		}

		[[nodiscard]] size_t getColorAttachmentsCount() const {
			return samples.size();
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;

			for(const auto& sample : samples) {
				sample->resize(w, h);
			}

			if(renderBuffer)renderBuffer->resize(w, h);
		}

		[[nodiscard]] bool check() const{
			bind();
			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		void bind() const{
			GL::bindFrameBuffer(targetFlag, bufferID);
		}

		void bind(const GLenum mode) const {
			GL::bindFrameBuffer(mode, bufferID);
		}

		void unbind() const{
			GL::bindFrameBuffer(targetFlag, 0);
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> readPixelsRaw(const unsigned int width, const unsigned int height, const int srcX = 0, const int srcY = 0) const {
			bind();
			auto pixels = std::make_unique<unsigned char[]>(static_cast<size_t>(width) * height * 4);
			glReadPixels(srcX, srcY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
			return pixels;
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> readPixelsRaw() const {
			return readPixelsRaw(width, height, 0, 0);
		}

		[[nodiscard]] const std::vector<std::unique_ptr<Texture2D>>& getTextures() const{
			return samples;
		}

		[[nodiscard]] std::vector<std::unique_ptr<Texture2D>>& getTextures(){
			return samples;
		}

		[[nodiscard]] const Texture2D* operator[](const size_t index) const{
			return samples.at(index).get();
		}

		void clear(const Graphic::Color& initColor = Graphic::Colors::CLEAR, const GLbitfield mask = GL_COLOR_BUFFER_BIT) const {
			bind();
			glClearColor(initColor.r, initColor.g, initColor.b, initColor.a);
			glClear(mask);
		}

		Texture2D& getTexture() const {
			return *samples.front().get();
		}

		[[nodiscard]] unsigned int getWidth() const { return width; }
		[[nodiscard]] unsigned int getHeight() const { return height; }

	};
}
