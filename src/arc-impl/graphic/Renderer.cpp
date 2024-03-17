module;

#include <glad/glad.h>

module Core.Renderer;

import Graphic.Draw;
import Assets.Graphic;
import RuntimeException;
import Core;
import Geom.Matrix3D;

void Core::Renderer::frameBegin(FrameBuffer* frameBuffer, bool resize, const Color& initColor, GLbitfield mask) {
	if (frameStack.top() == frameBuffer)throw ext::RuntimeException{ "Illegally Begin Twice!" };

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

void Core::Renderer::frameEnd(const ::std::function<void(FrameBuffer*, FrameBuffer*)>& func) {
	FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Draw::flush();

	func(contextFrameBuffer, beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd(PostProcessor* processor) {
	FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Draw::flush();

	processor->apply(contextFrameBuffer, beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd() {
	FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Draw::flush();

	contextFrameBuffer->getTextures().front()->active(0);
	contextFrameBuffer->bind(FrameBuffer::READ);
	Draw::blit(beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::renderUI() {
	const Geom::Matrix3D* mat = Core::overlayBatch->getProjection();

	Renderer::frameBegin(&uiPostBuffer);
	// Renderer::frameBegin(&uiBuffer);
	Core::overlayBatch->setProjection(Core::uiRoot->getPorj());

	Core::uiRoot->render();
	Core::overlayBatch->flush();

	const auto times = Assets::PostProcessors::bloom->blur.getProcessTimes();
	Assets::PostProcessors::bloom->blur.setProcessTimes(2);

	// frameEnd(Assets::PostProcessors::blendMulti);
	frameEnd(Assets::PostProcessors::bloom);

	Core::overlayBatch->setProjection(mat);
	Assets::PostProcessors::bloom->blur.setProcessTimes(times);
}

void Core::Renderer::resize(const unsigned w, const unsigned h) {
	if(sustainSize(w, h)) return;

	width  = w;
	height = h;

	defaultFrameBuffer.resize(w, h);
	// uiPostBuffer.resize(w + Core::uiRoot->marginX * 2, h + Core::uiRoot->marginY * 2);
	uiPostBuffer.resize(w, h);
	effectBuffer.resize(w, h);
	// uiBuffer.resize(w, h);

	contextFrameBuffer->resize(w, h);

	GL::viewport(w, h);

	for(const auto& resizeable : synchronizedSizedObjects) {
		resizeable->resize(w, h);
	}
}
