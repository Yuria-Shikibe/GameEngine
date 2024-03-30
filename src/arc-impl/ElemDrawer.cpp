module UI.ElemDrawer;

import UI.Elem;
import Graphic.Color;
import Graphic.Draw;
import RuntimeException;

void UI::TextureRegionRectDrawable::draw(const Geom::OrthoRectFloat rect) const {
#ifdef _DEBUG
	if(!texRegion)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
	Graphic::Draw::rectOrtho(texRegion, rect);
}

void UI::TextureNineRegionDrawable::draw(const Geom::OrthoRectFloat rect) const {
#ifdef _DEBUG
	if(!texRegion)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
	texRegion->render_RelativeExter(rect);
}

void UI::DrawPair::draw(const UI::Elem* elem, const float alphaScl, const Geom::OrthoRectFloat rect) const {
	Graphic::Draw::color<false>(color);
	Graphic::Draw::alpha(alphaScl * color.a);
	region->draw(rect);
}

void UI::UIStyle::drawElem(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	elem->tempColor.a *= elem->maskOpacity;

	float alphaScl = elem->selfMaskOpacity * elem->maskOpacity;

	Graphic::Draw::mixColor(elem->tempColor);
	const Rect rect = elem->getBound().setSrc(elem->getAbsSrc());
	base.draw(elem, alphaScl, rect);
	edge.draw(elem, alphaScl, rect);
	if(elem->isCursorInbound())inbound.draw(elem, alphaScl, rect);
	if(elem->isPressed())pressed.draw(elem, alphaScl, rect);

	Graphic::Draw::mixColor();
	//TODO disabled
	// if(elem->touchDisabled())disabled.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
}

void UI::StyleDrawer::applyToElem(Elem* elem) {
	elem->setBorder(style->margin);

	elem->changed();
}

void UI::EdgeDrawer::drawBackground(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	Graphic::Color& color = elem->tempColor;
	elem->tempColor.a *= elem->maskOpacity;
	Graphic::Draw::mixColor(elem->tempColor);

	if(elem->isCursorInbound()) {
		color.mul(1.1f).lerp(Graphic::Colors::WHITE, 0.3f);
		Graphic::Draw::color(color);
		Graphic::Draw::alpha(elem->isPressed() ? 0.5f : 0.2f);
		Graphic::Draw::rectOrtho(Graphic::Draw::defaultTexture, elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	}


	Graphic::Draw::color(color);
	Graphic::Draw::Line::setLineStroke(1.0f);
	Graphic::Draw::alpha(color.a * elem->maskOpacity);
	Graphic::Draw::Line::rectOrtho(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());

	Graphic::Draw::reset();
}

void UI::EmptyDrawer::applyToElem(Elem* elem){
	elem->setBorder(0.0f);
}
