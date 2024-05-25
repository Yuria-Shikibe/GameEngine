module;

#include <glad/glad.h>

module Core.Renderer;

import Graphic.Draw;
import Assets.Graphic;
import ext.RuntimeException;
import Core;
import Geom.Matrix3D;
import UI.SeperateDrawable;

using namespace Graphic;

//TODO should 'flush' at here?

void Core::Renderer::frameBegin(GL::FrameBuffer* frameBuffer, const bool resize, const Graphic::Color& initColor, GLbitfield mask) {
	if (contextFrameBuffer == frameBuffer)throw ext::RuntimeException{ "Illegally Begin Twice!" };

	Graphic::Batch::flush();

	frameStack.push(contextFrameBuffer);

	contextFrameBuffer = frameBuffer;

	if(resize){
		contextFrameBuffer->resize(width, height);
	}

	contextFrameBuffer->bind();
	contextFrameBuffer->enableDrawAll();

	contextFrameBuffer->clearColorAll(initColor);
	contextFrameBuffer->clearDepth(0.0f);
}

void Core::Renderer::frameBegin_Quiet(GL::FrameBuffer* frameBuffer, const bool resize){
	if (contextFrameBuffer == frameBuffer)throw ext::RuntimeException{ "Illegally Begin Twice!" };

	Graphic::Batch::flush();

	frameStack.push(contextFrameBuffer);

	contextFrameBuffer = frameBuffer;

	if(resize){
		contextFrameBuffer->resize(width, height);
	}

	contextFrameBuffer->bind();
	contextFrameBuffer->enableDrawAll();
}

void Core::Renderer::frameEnd(const ::std::function<void(GL::FrameBuffer*, GL::FrameBuffer*)>& func) {
	GL::FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Graphic::Batch::flush();

	func(contextFrameBuffer, beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd(const Graphic::PostProcessor* processor) {
	GL::FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Graphic::Batch::flush();

	processor->apply(contextFrameBuffer, beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd(const GL::ShaderSource* shader){
	GL::FrameBuffer* beneathFrameBuffer = frameBufferFallback();
	Graphic::Batch::flush();

	contextFrameBuffer->getColorAttachments().front()->active(0);
	Graphic::Frame::blit(beneathFrameBuffer, 0, shader, nullptr);

	contextFrameBuffer = beneathFrameBuffer;
}

void Core::Renderer::frameEnd() {
	GL::FrameBuffer* beneathFrameBuffer = frameBufferFallback();

	Graphic::Batch::flush();

	contextFrameBuffer->getColorAttachments().front()->active(0);
	contextFrameBuffer->bind(GL::FrameBuffer::READ);
	Graphic::Frame::blit(beneathFrameBuffer);

	contextFrameBuffer = beneathFrameBuffer;
}

GL::FrameBuffer* Core::Renderer::frameEnd_Quiet(){
	const auto cur = contextFrameBuffer;
	GL::FrameBuffer* beneathFrameBuffer = frameBufferFallback();
	Graphic::Batch::flush();

	contextFrameBuffer = beneathFrameBuffer;
	contextFrameBuffer->bind();
	return cur;
}


void Core::Renderer::processUISperateDraw(const UI::SeperateDrawable* drawable){
	GL::Blendings::Normal.apply();

	uiBlurMask.bind();
	uiBlurMask.clearColor();

	drawable->drawBase();
	Graphic::Batch::flush();
	Assets::PostProcessors::frostedGlassBlur->apply(contextFrameBuffer, &effectBuffer);


	effectBuffer.getTopTexture().active(0);
	contextFrameBuffer->getTopTexture().active(1);
	uiBlurMask.getTopTexture().active(2);

	GL::disable(GL::State::BLEND);
	Graphic::Frame::blit(contextFrameBuffer, 0, Assets::Shaders::mask, [](const GL::ShaderProgram& shader){
		shader.setColor("mixColor", Colors::AQUA_SKY.createLerp(Colors::DARK_GRAY, 0.55f).setA(0.25f));
		shader.setColor("srcColor", Colors::GRAY.createLerp(Colors::DARK_GRAY, 0.35f).createLerp(Colors::AQUA_SKY, 0.125f));
	});
	GL::enable(GL::State::BLEND);

	Renderer::frameBegin(&uiPostBuffer);
	drawable->draw();
	frameEnd(Assets::PostProcessors::bloom.get());
}

//TODO merge these two function

void Core::Renderer::renderUI() {
	if(Core::uiRoot->isHidden)return;
	const Geom::Matrix3D* mat = Graphic::Batch::getPorj();
	Core::batchGroup.overlay->setProjection(Core::uiRoot->getPorj());

	const auto times = Assets::PostProcessors::bloom->blur.getProcessTimes();
	Assets::PostProcessors::bloom->blur.setProcessTimes(2);

	processUISperateDraw(Core::uiRoot->currentScene);
	processUISperateDraw(&Core::uiRoot->rootDialog);

	const auto [dropped, focused] = Core::uiRoot->tooltipManager.getDrawSeq();

	for (auto&& toDraw : focused){
		processUISperateDraw(toDraw);
	}

	for (auto&& toDraw : dropped){
		processUISperateDraw(toDraw);
	}

	batchGroup.overlay->setProjection(mat);
	Assets::PostProcessors::bloom->blur.setProcessTimes(times);
}

void Core::Renderer::resize(const int w, const int h) {
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

	GL::viewport(w, h);

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

	renderUI();

	Graphic::Draw::color();
	Graphic::Draw::mixColor();

	drawControlHook.fire(draw_overlay);

	glBlitNamedFramebuffer(defaultFrameBuffer.getID(), 0,
	                       0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST
	);
}
