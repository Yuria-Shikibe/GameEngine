module;

export module GL.Buffer.MultiSampleFrameBuffer;

import <glad/glad.h>;

import GL.Buffer.FrameBuffer;
import GL.Buffer.MultiSampleRenderBuffer;
import GL.Texture.MultiSampleTexture2D;

export namespace GL {
	class MultiSampleFrameBuffer final : virtual public FrameBuffer {
	public:
		MultiSampleFrameBuffer(const unsigned int w, const unsigned int h, const int samples)
				: FrameBuffer() {
			sample.reset(new MultiSampleTexture2D{w, h, samples});
			renderBuffer.reset(new MultiSampleRenderBuffer{w, h, samples});
			width = w;
			height = h;

			targetFlag = GL_FRAMEBUFFER;

			glGenFramebuffers(1, &bufferID);
			FrameBuffer::bind();

			glFramebufferTexture2D(targetFlag, GL_COLOR_ATTACHMENT0, sample->getTargetFlag(), sample->getID(), 0);
			glFramebufferRenderbuffer(targetFlag, GL_DEPTH_STENCIL_ATTACHMENT, renderBuffer->getTargetFlag(), renderBuffer->getID());

			// sample->setScale(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

			FrameBuffer::unbind();
		}

		MultiSampleFrameBuffer(const unsigned int w, const unsigned int h) : MultiSampleFrameBuffer(w, h, 4) {

		}

		[[nodiscard]] MultiSampleFrameBuffer() = default;

		using FrameBuffer::resize;
	};
}


