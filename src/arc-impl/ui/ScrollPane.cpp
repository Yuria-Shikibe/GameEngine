module UI.ScrollPane;

import UI.Drawer;
import Graphic.Draw;

import UI.Root;
import Graphic.Color;
import Math;

using UI::Root;

void UI::ScrollBarDrawer::operator()(const ScrollPane* pane) const {
	if(pane->isPressed()){
		Graphic::Draw::color(pressedBarColor);
	}else{
		Graphic::Draw::color(barColor);
	}

	if(pane->enableHorizonScroll()) {
		region.render_RelativeExter(pane->getHoriBarRect().copy().shrink(margin).moveY(pane->getBorder().bottom * -offsetScl.x));
	}

	if(pane->enableVerticalScroll()) {
		region.render_RelativeExter(pane->getVertBarRect().copy().shrink(margin).moveX(pane->getBorder().right * offsetScl.y));
	}
}

void UI::ScrollPane::update(const float delta){
	if(Widget::isInbound(root->cursorPos) && (enableHorizonScroll() || enableVerticalScroll())) {
		Widget::setFocusedScroll(true);
	}else{
		Widget::setFocusedScroll(false);
	}



	scrollVelocity.lerp(scrollTargetVelocity, usingAccel ? (pressed ? 1.0f : Math::clamp(accel * delta)) : 1.0f);

	Group::update(delta);

	if(scrollTempOffset != scrollOffset){
		//TODO what...?
		if(hasChildren()) {
			itemSize = getItem()->getBound();
			getItem()->layout_tryFillParent();
		}
	}else{
		scrollOffset.add(scrollVelocity);
		clamp(scrollOffset);

		if(hasChildren()){
			const float deltaH = getItem()->getHeight() - itemSize.getHeight();
			const float deltaW = getItem()->getWidth() - itemSize.getWidth();

			if(deltaH < getHeight())scrollOffset.y += deltaH;
			if(deltaW < getWidth())scrollOffset.x += deltaW;

			itemSize = getItem()->getBound();
			getItem()->layout_tryFillParent();
		}

		clamp(scrollOffset);
		resumeTemp();
	}

	const float ratioX = horiScrollRatio(-scrollOffset.x);
	const float ratioY = vertScrollRatio(scrollOffset.y);
	constexpr float triggerVal = 0.005f;
	if(ratioX > 1.0f - triggerVal || ratioX < triggerVal){
		scrollVelocity.x = 0;
	}

	if(ratioY > 1.0f - triggerVal || ratioY < triggerVal){
		scrollVelocity.y = 0;
	}

	scrollTargetVelocity.setZero();

	if(layoutChanged) {
		layout();
	}

	calAbsoluteSrc(parent);
}

UI::CursorType UI::ScrollPane::getCursorType() const{
	if(root){
		if(isInHoriBar(root->cursorPos)){
			return CursorType::scrollHori;
		}

		if(isInVertBar(root->cursorPos)){
			return CursorType::scrollVert;
		}
	}
	return CursorType::scroll;
}

void UI::ScrollPane::applyDefDrawer(){
	Group::applyDefDrawer();
	scrollBarDrawer = &UI::defScrollBarDrawer;
}

void UI::ScrollPane::drawBase() const{
	Widget::drawBase();
}

void UI::ScrollPane::drawContent() const{
	Graphic::Batch::flush();

	const auto lastRect = GL::getScissorRect();

	const Geom::OrthoRectInt clip{
		Math::trac(absoluteSrc.x + border.left), Math::trac(absoluteSrc.y + horiBarStroke() + border.bottom),
	Math::ceilPositive(getContentWidth()), Math::ceilPositive(getContentHeight())
	};

	const auto count = GL::getSrhinkCount();
	GL::enable(GL::Test::SCISSOR);

	if(!count)GL::forceSetScissor(clip);
	GL::scissorShrinkBegin();
	GL::setScissor(clip);

	drawChildren();
	Graphic::Batch::flush();

	GL::forceSetScissor(lastRect);
	GL::scissorShrinkEnd();
	if(!count)GL::disable(GL::Test::SCISSOR);

	scrollBarDrawer->operator()(this);
}
