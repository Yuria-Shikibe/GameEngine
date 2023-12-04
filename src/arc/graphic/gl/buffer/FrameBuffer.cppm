module;

export module GL.Buffer.FrameBuffer;

import <glad/glad.h>;
import <memory>;

import GL.Texture.Texture2D;
import GL.Buffer;
import GL.Buffer.RenderBuffer;
import Graphic.Resizeable;
import Graphic.Color;

export namespace GL{
	class FrameBuffer : virtual public GLBuffer, virtual public Graphic::ResizeableInt
	{
	protected:
		unsigned int width = 0, height = 0;
		std::unique_ptr<Texture2D> sample{nullptr};
		std::unique_ptr<RenderBuffer> renderBuffer{nullptr};

	public:
		static constexpr GLenum DEF = GL_FRAMEBUFFER;
		static constexpr GLenum DRAW = GL_DRAW_FRAMEBUFFER;
		static constexpr GLenum READ = GL_READ_FRAMEBUFFER;

		FrameBuffer() = default;

		FrameBuffer(const unsigned int w, const unsigned int h){
			sample = std::make_unique<Texture2D>(w, h);
			sample->localData.reset(nullptr);
			renderBuffer = std::make_unique<RenderBuffer>(w, h);
			width = w;
			height = h;

			targetFlag = GL_FRAMEBUFFER;

			glGenFramebuffers(1, &bufferID);
			FrameBuffer::bind();

			glFramebufferTexture2D(targetFlag, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, sample->getID(), 0);
			glFramebufferRenderbuffer(targetFlag, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer->getID());

			FrameBuffer::unbind();
		}

		~FrameBuffer() override{
			glDeleteFramebuffers(1, &bufferID);
		}

		void resize(const unsigned int w, const unsigned int h) override{
			width = w;
			height = h;
			if(sample)sample->resize(w, h);
			if(renderBuffer)renderBuffer->resize(w, h);
		}

		[[nodiscard]] bool check() const{
			bind();
			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		void bind() const override{
			glBindFramebuffer(targetFlag, bufferID);
		}

		void bind(const GLenum mode) const {
			glBindFramebuffer(mode, bufferID);
		}

		void unbind() const override{
			glBindFramebuffer(targetFlag, 0);
		}

		[[nodiscard]] unsigned char* readPixelsRaw(const unsigned int width, const unsigned int height, const int srcX = 0, const int srcY = 0) const {
			bind();
			auto* pixels = new unsigned char[width * height * 4]{0};
			glReadPixels(srcX, srcY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);



			return pixels;
		}

		[[nodiscard]] unsigned char* readPixelsRaw() const {
			return readPixelsRaw(width, height, 0, 0);
		}


		[[nodiscard]] Texture2D& getTexture(const bool loadData) const{
			if(loadData) {
				sample->updateData();
			}

			return *sample;
		}

		[[nodiscard]] Texture2D& getTexture() const{
			return *sample;
		}

		[[nodiscard]] GLuint getTextureID() const{
			return sample->getID();
		}

		void clear(const Graphic::Color& initColor = Graphic::Colors::CLEAR, const GLbitfield mask = GL_COLOR_BUFFER_BIT) const {
			bind();
			glClearColor(initColor.r, initColor.g, initColor.b, initColor.a);
			glClear(mask);
		}


		[[nodiscard]] unsigned int getWidth() const { return width; }
		[[nodiscard]] unsigned int getHeight() const { return height; }

	};
}
