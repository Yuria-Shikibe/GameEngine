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

	Graphic::Batch::flush();

	frameStack.push(contextFrameBuffer);

	contextFrameBuffer = frameBuffer;

	if(resize){
		contextFrameBuffer->resize(width, height);
	}

	contextFrameBuffer->bind();
	GL::viewport(contextFrameBuffer->getWidth(), contextFrameBuffer->getHeight());


	contextFrameBuffer->enableDrawAll();

	contextFrameBuffer->clearColorAll(initColor);
	contextFrameBuffer->clearDepth();
	glClearColor(initColor.r, initColor.g, initColor.b, initColor.a);
	glClearDepth(0.0f); //TODO why?????????
	glClear(mask);
}

void Core::Renderer::frameEnd(const ::std::function<void(FrameBuffer*, FrameBuffer*)>& func) {
	FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Graphic::Batch::flush();

	func(contextFrameBuffer, beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd(const PostProcessor* processor) {
	FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Graphic::Batch::flush();

	processor->apply(contextFrameBuffer, beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd() {
	FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Graphic::Batch::flush();

	contextFrameBuffer->getTextures().front()->active(0);
	contextFrameBuffer->bind(FrameBuffer::READ);
	Graphic::Frame::blit(beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::renderUI() {
	const Geom::Matrix3D* mat = Graphic::Batch::getPorj();

	Renderer::frameBegin(&uiPostBuffer);
	// Renderer::frameBegin(&uiBuffer);
	Core::batchGroup.batchOverlay->setProjection(Core::uiRoot->getPorj());

	Core::uiRoot->render();
	Graphic::Batch::flush();

	const auto times = Assets::PostProcessors::bloom->blur.getProcessTimes();
	Assets::PostProcessors::bloom->blur.setProcessTimes(2);

	// frameEnd(Assets::PostProcessors::blendMulti);
	frameEnd(Assets::PostProcessors::bloom.get());

	Core::batchGroup.batchOverlay->setProjection(mat);
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
