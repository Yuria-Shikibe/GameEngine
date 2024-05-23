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
		MultiSampleFrameBuffer(const int w, const int h, const int samples = 4, const int colorAttachments = 1, const bool hasRenderBuffer = false)
				: FrameBuffer(), samples(samples) {
			width = w;
			height = h;
			glCreateFramebuffers(1, &nameID);

			setColorAttachments<MultiSampleTexture2D>(colorAttachments, samples);
			if(hasRenderBuffer)setRenderBuffer<MultiSampleRenderBuffer>(samples);
		}

		[[nodiscard]] MultiSampleFrameBuffer() = default;

		void resize(const int w, const int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;

			setColorAttachments<MultiSampleTexture2D>(getColorAttachmentsCount(), samples);

			if(renderBuffer)renderBuffer->resize(w, h);
		}
	};
}


