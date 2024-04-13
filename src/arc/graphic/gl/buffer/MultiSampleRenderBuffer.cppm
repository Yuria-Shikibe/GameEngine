module;

#include <glad/glad.h>

export module GL.Buffer.MultiSampleRenderBuffer;
import GL.Buffer.RenderBuffer;

export namespace GL{
	class MultiSampleRenderBuffer final : public RenderBuffer{
	public:
		int samples = 4;

		MultiSampleRenderBuffer(const unsigned int w, const unsigned int h, const int samples) : RenderBuffer() {
			this->samples = samples;

			width = w;
			height = h;

			glCreateRenderbuffers(1, &nameID);
			glNamedRenderbufferStorageMultisample(nameID, samples, GL_DEPTH32F_STENCIL8, w, h);
		}

		void resize(const int w, const int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;
			glNamedRenderbufferStorageMultisample(nameID, samples, GL_DEPTH32F_STENCIL8, w, h);
		}
	};
}


