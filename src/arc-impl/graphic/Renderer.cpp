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

	contextFrameBuffer->getColorAttachments().front()->active(0);
	contextFrameBuffer->bind(FrameBuffer::READ);
	Graphic::Frame::blit(beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}


//TODO merge these two function
void Core::Renderer::renderUIBelow() {
	const Geom::Matrix3D* mat = Graphic::Batch::getPorj();

	Core::batchGroup.batchOverlay->setProjection(Core::uiRoot->getPorj());

	uiBlurMask.bind();
	uiBlurMask.clearColor();
	Core::uiRoot->renderBase();
	Graphic::Batch::flush();

	Assets::PostProcessors::blur_Far->apply(contextFrameBuffer, &effectBuffer);

	uiBlurMask.getMaskBuffer().getColorAttachments().front()->active(2);
	contextFrameBuffer->getColorAttachments().front()->active(1);
	effectBuffer.getColorAttachments().front()->active(0);

	Graphic::Frame::blit(contextFrameBuffer, 0, Assets::Shaders::mask, [](const GL::Shader& shader){
		shader.setColor("mixColor", Colors::AQUA_SKY.createLerp(Colors::LIGHT_GRAY, 0.75f).setA(0.75f));
		shader.setColor("srcColor", Colors::AQUA_SKY.createLerp(Colors::LIGHT_GRAY, 0.856f));
	});

	Renderer::frameBegin(&uiPostBuffer);

	uiRoot->render();
	Graphic::Batch::flush();

	const auto times = Assets::PostProcessors::bloom->blur.getProcessTimes();
	Assets::PostProcessors::bloom->blur.setProcessTimes(2);

	frameEnd(Assets::PostProcessors::bloom.get());

	batchGroup.batchOverlay->setProjection(mat);
	Assets::PostProcessors::bloom->blur.setProcessTimes(times);
}

void Core::Renderer::renderUIAbove(){
	const Geom::Matrix3D* mat = Graphic::Batch::getPorj();

	batchGroup.batchOverlay->setProjection(uiRoot->getPorj());

	uiBlurMask.bind();
	uiBlurMask.clearColor();
	uiRoot->renderBaseAbove();
	Graphic::Batch::flush();

	Assets::PostProcessors::blur_Far->apply(contextFrameBuffer, &effectBuffer);

	uiBlurMask.getMaskBuffer().getColorAttachments().front()->active(2);
	contextFrameBuffer->getColorAttachments().front()->active(1);
	effectBuffer.getColorAttachments().front()->active(0);

	Graphic::Frame::blit(contextFrameBuffer, 0, Assets::Shaders::mask, [](const GL::Shader& shader){
		shader.setColor("mixColor", Colors::AQUA_SKY.createLerp(Colors::LIGHT_GRAY, 0.75f).setA(0.75f));
		shader.setColor("srcColor", Colors::AQUA_SKY.createLerp(Colors::LIGHT_GRAY, 0.856f));
	});

	Renderer::frameBegin(&uiPostBuffer);

	uiRoot->renderAbove();
	Graphic::Batch::flush();

	const auto times = Assets::PostProcessors::bloom->blur.getProcessTimes();
	Assets::PostProcessors::bloom->blur.setProcessTimes(2);

	frameEnd(Assets::PostProcessors::bloom.get());

	batchGroup.batchOverlay->setProjection(mat);
	Assets::PostProcessors::bloom->blur.setProcessTimes(times);
}

void Core::Renderer::resize(const unsigned w, const unsigned h) {
	if(sustainSize(w, h)) return;

	width  = w;
	height = h;

	defaultFrameBuffer.resize(w, h);
	// uiPostBuffer.resize(w + Core::uiRoot->marginX * 2, h + Core::uiRoot->marginY * 2);
	uiPostBuffer.resize(w, h);
	uiBlurMask.resize(w, h);
	effectBuffer.resize(w, h);
	// uiBuffer.resize(w, h);

	contextFrameBuffer->resize(w, h);

	GL::viewport(static_cast<int>(w), static_cast<int>(h));

	for(const auto& resizeable : synchronizedSizedObjects) {
		resizeable->resize(w, h);
	}
}

void Core::Renderer::draw(){
	defaultFrameBuffer.bind();
	GL::viewport(static_cast<int>(width), static_cast<int>(height));

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	drawControlHook.fire(draw_prepare);

	drawMain();

	drawControlHook.fire(draw_post);

	drawControlHook.fire(draw_after);

	renderUIBelow();

	renderUIAbove();

	Graphic::Draw::color();
	Graphic::Draw::mixColor();

	drawControlHook.fire(draw_overlay);

	glBlitNamedFramebuffer(defaultFrameBuffer.getID(), 0,
	                       0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST
	);
}
