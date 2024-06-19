module UI.Screen;

import Graphic.Draw;
import GL;
import Core;
import Assets.Graphic;


void UI::Viewport::endDraw_noContextFallback() const{

	if(batchPtr){
		(Core::batchGroup.*batchPtr)->flush();
		(Core::batchGroup.*batchPtr)->setProjection(fallbackMat);
		fallbackMat = nullptr;
		batchPtr = nullptr;
	}else{
		if(fallbackMat){
			throw ext::RuntimeException{"Batch Should Be Specified On End"};
		}
	}

	unlockViewport();
	//TODO this may cause bug, should be careful
	//TODO Reserve original framebuffer id or pointer?
	buffer.unbind();
}

void UI::Viewport::draw() const{
	Elem::draw();
}

void UI::Viewport::beginDraw(std::unique_ptr<Core::Batch> Core::BatchGroup::* batchPtr) const{
	this->batchPtr = batchPtr;

	if(batchPtr){
		(Core::batchGroup.*batchPtr)->flush();
		fallbackMat = (Core::batchGroup.*batchPtr)->getProjection();
		(Core::batchGroup.*batchPtr)->setProjection(camera.getWorldToScreen());
	}

	buffer.bind();
	buffer.enableDrawAll();
	buffer.clearColorAll();
	lockViewport();

}

void UI::Viewport::draw_OrthoPort(){
	matrix.setOrthogonal(0, 0, getValidWidth(), getValidHeight());
	if(batchPtr){
		(Core::batchGroup.*batchPtr)->flush();
		(Core::batchGroup.*batchPtr)->setProjection(matrix);
	}
}

void UI::Viewport::draw_CameraPort(){
	if(batchPtr){
		(Core::batchGroup.*batchPtr)->flush();
		(Core::batchGroup.*batchPtr)->setProjection(camera.getWorldToScreen());
	}
}

void UI::Viewport::endDraw() const{
	endDraw_noContextFallback();

	Core::renderer->contextFrameBuffer->bind();
	Core::renderer->contextFrameBuffer->enableDrawAll();
}

void UI::Viewport::drawContent() const{
	using namespace Graphic;

	{
		[[maybe_unused]] Core::BatchGuard_Proj bg{*Core::batchGroup.overlay};
		bg.current.setOrthogonal(absoluteSrc.x, absoluteSrc.y, getWidth(), getHeight());

		buffer.bind();
		buffer.enableDrawAt(1);
		drawBase();
	}

	GL::FrameBuffer* cur = nullptr;
	if(!usesUIEffect){
		cur = Core::renderer->frameEnd_Quiet();
	}

	buffer.getColorAttachments().at(0)->active(0);
	Core::renderer->contextFrameBuffer->getTopTexture().active(1);
	buffer.getColorAttachments().at(1)->active(2);

	Core::renderer->contextFrameBuffer->bind();
	Core::renderer->contextFrameBuffer->enableDrawAt(0);

	GL::viewport(absoluteSrc.x, absoluteSrc.y, getWidth(), getHeight());

	GL::disable(GL::State::BLEND);
	Assets::Shaders::mask->bind();
	Assets::Shaders::mask->applyDynamic([](const GL::ShaderProgram& shader){
		shader.setColor("mixColor", Colors::CLEAR);
		shader.setColor("srcColor", Colors::WHITE);
	}, true);

	Frame::rawMesh->bind();
	Frame::rawMesh->render(GL_TRIANGLE_FAN, 0, GL::ELEMENTS_QUAD_STRIP_LENGTH);
	GL::enable(GL::State::BLEND);

	if(cur)Core::renderer->frameBegin_Quiet(cur);
	else{
		Core::renderer->contextFrameBuffer->bind();
	}
}

void UI::Viewport::beginCameraFocus(){
	Core::focus.camera.switchFocus(&camera);
}

void UI::Viewport::endCameraFocus() const{
	if(Core::focus.camera.operator->() == &camera)Core::focus.camera.switchDef();
}

void UI::Viewport::lockViewport() const noexcept{
	GL::lockViewport = true;
}

void UI::Viewport::unlockViewport() const noexcept{
	GL::lockViewport = false;
}

Core::Batch* UI::Viewport::getBatch() const{
	if(batchPtr){
		return (Core::batchGroup.*batchPtr).get();
	}
	return nullptr;
}
