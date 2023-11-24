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
		int width = 0, height = 0;
		Texture2D* sample = nullptr;
		RenderBuffer* renderBuffer = nullptr;

	public:
		FrameBuffer() = default;

		FrameBuffer(const int w, const int h){
			sample = new Texture2D{ w, h };
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

		void resize(const int w, const int h) override{
			width = w;
			height = h;
			if(sample)sample->resize(w, h);
			if(renderBuffer)renderBuffer->resize(w, h);
		}

		bool check() const{
			bind();
			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		void bind() const override{
			glBindFramebuffer(targetFlag, bufferID);
		}

		void unbind() const override{
			glBindFramebuffer(targetFlag, 0);
		}

		Texture2D& getTexture() const{
			return *sample;
		}

		GLuint getTextureID() const{
			return sample->getID();
		}


		int getWidth() const { return width; }
		int getHeight() const { return height; }

	};
}
