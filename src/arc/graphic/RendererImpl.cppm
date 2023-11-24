//
// Created by Matrix on 2023/11/23.
//
module;

export module Graphic.RendererImpl;

import <glad/glad.h>;
import GL.GL_Exception;
import GL.Constants;
import Core.Renderer;
import GL.Buffer.FrameBuffer;
import Graphic.Color;
import Graphic.Draw;

using namespace GL;

export namespace Graphic {
	class RendererImpl final : public virtual Core::Renderer {
	public:
		RendererImpl(const int w, const int h)
			: Renderer(w, h) {
		}

		void frameBegin(GL::FrameBuffer& frameBuffer, const bool resize = false, const Color& initColor = Colors::CLEAR, const GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) override {
			{
				if (frameStack.top() == &frameBuffer)throw GL_Exception{ "Illegally Begin Twice!" };

				Draw::flush();

				frameStack.push(contextFrameBuffer);

				contextFrameBuffer = &frameBuffer;

				if(resize){
					contextFrameBuffer->resize(width, height);
				}

				contextFrameBuffer->bind();

				glClearColor(initColor.r, initColor.g, initColor.b, initColor.a);
				glClear(mask);
			}
		}

		void frameEnd() override {
			{
				FrameBuffer* beneathFrameBuffer = frameBufferFallback();

				Draw::flush();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, contextFrameBuffer->getID());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, beneathFrameBuffer->getID());
				glBlitFramebuffer(0, 0, contextFrameBuffer->getWidth(), contextFrameBuffer->getHeight(), 0, 0, beneathFrameBuffer->getWidth(), beneathFrameBuffer->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);

				contextFrameBuffer = beneathFrameBuffer;
			}
		}
	};
}