module UI.ElemDrawer;

import UI.Elem;
import Graphic.Color;
import Graphic.Draw;
import RuntimeException;

void UI::TextureRegionRectDrawable::draw(const float srcx, const float srcy, const float width, const float height) const {
#ifdef _DEBUG
	if(!rect)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
	Graphic::Draw::rectOrtho(rect, srcx, srcy, width, height);
}

void UI::TextureNineRegionDrawable::draw(float srcx, float srcy, float width, float height) const {
#ifdef _DEBUG
	if(!rect)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
	rect->render_RelativeExter(srcx, srcy, width, height);
}

void UI::DrawPair::draw(const float srcx, const float srcy, const float width, const float height) const {
	Graphic::Draw::color(color);
	region->draw(srcx, srcy, width, height);
}

void UI::UIStyle::drawElem(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	elem->tempColor.a *= elem->maskOpacity;
	Graphic::Draw::mixColor(elem->tempColor);

	base.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	edge.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	if(elem->isPressed())pressed.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	if(elem->cursorInbound())inbound.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());

	Graphic::Draw::mixColor();
	//TODO disabled
	// if(elem->touchDisabled())disabled.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
}

void UI::StyleDrawer::applyToElem(Elem* elem) {
	elem->margin_bottomLeft.x = style->margin_left;
	elem->margin_bottomLeft.y = style->margin_bottom;

	elem->margin_topRight.x = style->margin_right;
	elem->margin_topRight.y = style->margin_top;

	elem->changed();
}

void UI::EdgeDrawer::drawBackground(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	Graphic::Color& color = elem->tempColor;
	elem->tempColor.a *= elem->maskOpacity;
	Graphic::Draw::mixColor(elem->tempColor);

	if(elem->cursorInbound()) {
		color.mul(1.1f).lerp(Graphic::Colors::WHITE, 0.3f);
		Graphic::Draw::color(color);
		Graphic::Draw::alpha(elem->isPressed() ? 0.5f : 0.2f);
		Graphic::Draw::rectOrtho(Graphic::Draw::defaultTexture, elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	}


	Graphic::Draw::color(color);
	Graphic::Draw::Line::setLineStroke(1.0f);
	Graphic::Draw::alpha(color.a * elem->maskOpacity);
	Graphic::Draw::Line::rect(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());

	Graphic::Draw::reset();
}
