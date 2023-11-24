module;

export module GL.Buffer.MultiSampleFrameBuffer;

import <glad/glad.h>;

import GL.Buffer.FrameBuffer;
import GL.Buffer.MultiSampleRenderBuffer;
import GL.Texture.MultiSampleTexture2D;

export namespace GL {
	class MultiSampleFrameBuffer final : virtual public FrameBuffer {
	public:
		MultiSampleFrameBuffer(const int w, const int h, const int texSamples, const int renderSamples)
				: FrameBuffer() {
			sample = new MultiSampleTexture2D{w, h, texSamples};
			renderBuffer = new MultiSampleRenderBuffer{w, h, renderSamples};
			width = w;
			height = h;

			targetFlag = GL_FRAMEBUFFER;

			glGenFramebuffers(1, &bufferID);
			FrameBuffer::bind();

			glFramebufferTexture2D(targetFlag, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, sample->getID(), 0);
			glFramebufferRenderbuffer(targetFlag, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer->getID());

			FrameBuffer::unbind();
		}

		MultiSampleFrameBuffer(const int w, const int h) : MultiSampleFrameBuffer(w, h, 4, 4) {

		}
	};
}


