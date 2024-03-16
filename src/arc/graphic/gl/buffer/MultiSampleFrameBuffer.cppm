module;

export module GL.Buffer.MultiSampleFrameBuffer;

import <glad/glad.h>;
import GL.Buffer.FrameBuffer;
import GL.Buffer.MultiSampleRenderBuffer;
import GL.Texture.MultiSampleTexture2D;

export namespace GL {
	class MultiSampleFrameBuffer final : public FrameBuffer {
	public:
		MultiSampleFrameBuffer(const unsigned int w, const unsigned int h, const int samples = 4, const int colorAttachments = 1)
				: FrameBuffer() {
			width = w;
			height = h;
			glCreateFramebuffers(1, &nameID);

			bindColorAttachments<MultiSampleTexture2D>(colorAttachments, samples);
			bindRenderBuffer<MultiSampleRenderBuffer>(samples);
		}

		[[nodiscard]] MultiSampleFrameBuffer() = default;
	};
}


