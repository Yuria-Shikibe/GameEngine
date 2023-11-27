module;

export module GL.Buffer.FrameBuffer;

import <glad/glad.h>;

import GL.Texture.Texture2D;
import GL.Buffer;
import GL.Buffer.RenderBuffer;
import Graphic.Resizeable;

export namespace GL{
	class FrameBuffer : virtual public GLBuffer, virtual public Graphic::ResizeableInt
	{
	protected:
		unsigned int width = 0, height = 0;
		Texture2D* sample = nullptr;
		RenderBuffer* renderBuffer = nullptr;

	public:
		FrameBuffer() = default;

		FrameBuffer(const unsigned int w, const unsigned int h){
			sample = new Texture2D{ w, h };
			sample->localData.reset(nullptr);
			renderBuffer = new RenderBuffer{ w, h };
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

			delete sample;
			delete renderBuffer;
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

		void unbind() const override{
			glBindFramebuffer(targetFlag, 0);
		}

		[[nodiscard]] unsigned char* readPixelsRaw(const int width, const int height, const int srcX = 0, const int srcY = 0) const {
			bind();
			auto* pixels = new unsigned char[width * height * 4]{0};
			glReadPixels(srcX, srcY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);



			return pixels;
		}

		[[nodiscard]] unsigned char* readPixelsRaw() const {
			return readPixelsRaw(width, height, 0, 0);
		}


		[[nodiscard]] Texture2D& getTexture(const bool loadData = true) const{
			// if(loadData)sample->localData.reset(readPixelsRaw());
			if(loadData) {
				sample->updateData();
			}

			return *sample;
		}

		[[nodiscard]] GLuint getTextureID() const{
			return sample->getID();
		}


		[[nodiscard]] int getWidth() const { return width; }
		[[nodiscard]] int getHeight() const { return height; }

	};
}
