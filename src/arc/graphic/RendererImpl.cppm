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
import Graphic.PostProcessor;
import Graphic.Color;
import Graphic.Draw;

import Assets.Graphic;

import Core;
import Geom.Matrix3D;

using namespace GL;

export namespace Graphic {
	class RendererImpl : public Core::Renderer {
	public:
		GL::FrameBuffer uiBuffer;

		RendererImpl(const unsigned int w, const unsigned int h)
			: Renderer(w, h), uiBuffer{w, h} {

			Renderer::registerSynchronizedResizableObject(&uiBuffer);
		}

		void frameBegin(GL::FrameBuffer* frameBuffer, const bool resize, const Color& initColor, const GLbitfield mask) override {
			{
				if (frameStack.top() == frameBuffer)throw GL_Exception{ "Illegally Begin Twice!" };

				Draw::flush();

				frameStack.push(contextFrameBuffer);

				contextFrameBuffer = frameBuffer;

				if(resize){
					contextFrameBuffer->resize(width, height);
				}

				contextFrameBuffer->bind();
				GL::viewport(contextFrameBuffer->getWidth(), contextFrameBuffer->getHeight());

				glClearColor(initColor.r, initColor.g, initColor.b, initColor.a);
				glClear(mask);
			}
		}

		void frameEnd(PostProcessor* processor) override {
			{
				FrameBuffer* beneathFrameBuffer = frameBufferFallback();

				Draw::flush();

				processor->apply(contextFrameBuffer, beneathFrameBuffer);

				contextFrameBuffer = beneathFrameBuffer;
			}
		}

		void frameEnd() override {
			{
				FrameBuffer* beneathFrameBuffer = frameBufferFallback();

				Draw::flush();

				Draw::blit(beneathFrameBuffer);

				contextFrameBuffer = beneathFrameBuffer;
			}
		}

		void renderUI() override {
			const Geom::Matrix3D* mat = Core::batch->getProjection();

			Renderer::frameBegin(&uiBuffer);
			Core::batch->setProjection(Core::uiRoot->getPorj());

			Core::uiRoot->render();
			Core::batch->flush();

			Assets::PostProcessors::bloom->blur.setProcessTimes(1);
			frameEnd(Assets::PostProcessors::bloom);
			Core::batch->setProjection(mat);
			Assets::PostProcessors::bloom->blur.setProcessTimes(3);
		}
	};
}