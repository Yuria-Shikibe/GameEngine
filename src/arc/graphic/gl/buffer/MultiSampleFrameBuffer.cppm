module;

#include <glad/glad.h>

export module GL.Buffer.MultiSampleFrameBuffer;

import GL.Buffer.FrameBuffer;
import GL.Buffer.MultiSampleRenderBuffer;
import GL.Texture.MultiSampleTexture2D;

export namespace GL {
	class MultiSampleFrameBuffer final : public FrameBuffer {
	public:
		int samples{4};
		MultiSampleFrameBuffer(const unsigned int w, const unsigned int h, const int samples = 4, const int colorAttachments = 1)
				: FrameBuffer(), samples(samples) {
			width = w;
			height = h;
			glCreateFramebuffers(1, &nameID);

			setColorAttachments<MultiSampleTexture2D>(colorAttachments, samples);
			setRenderBuffer<MultiSampleRenderBuffer>(samples);
		}

		[[nodiscard]] MultiSampleFrameBuffer() = default;

		void resize(const unsigned int w, const unsigned int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;

			setColorAttachments<MultiSampleTexture2D>(getColorAttachmentsCount(), samples);

			if(renderBuffer)renderBuffer->resize(w, h);
		}
	};
}


