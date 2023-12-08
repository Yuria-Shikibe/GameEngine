module;

export module GL.Buffer.MultiSampleRenderBuffer;

import <glad/glad.h>;

import GL.Buffer.RenderBuffer;

export namespace GL{
	class MultiSampleRenderBuffer final : public RenderBuffer{
	public:
		int samples = 4;

		MultiSampleRenderBuffer(const unsigned int w, const unsigned int h, const int samples_) : RenderBuffer() {
			targetFlag = GL_RENDERBUFFER;
			this->samples = samples_;

			width = w;
			height = h;

			glGenRenderbuffers(1, &bufferID);
			RenderBuffer::bind();
			glRenderbufferStorageMultisample(targetFlag, samples_, GL_DEPTH24_STENCIL8, w, h);
			RenderBuffer::unbind();
		}

		void resize(const unsigned int w, const unsigned int h) override{
			width = w;
			height = h;
			bind();
			glRenderbufferStorageMultisample(targetFlag, samples, GL_DEPTH24_STENCIL8, w, h);
			unbind();
		}
	};
}


