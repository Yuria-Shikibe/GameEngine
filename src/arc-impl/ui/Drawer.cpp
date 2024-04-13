module UI.Drawer;

import UI.Elem;
import Graphic.Color;
import Graphic.Draw;
import RuntimeException;
import UI.ProgressBar;
import UI.SliderBar;

using namespace Graphic;

void UI::SliderBarDrawer::draw(const SliderBar* sliderBar) const{
	//TODO trans these into the slide bar drawer
	Rect rect = sliderBar->getValidBound().move(sliderBar->getAbsSrc());

	Draw::color(Colors::LIGHT_GRAY);
	Draw::alpha(0.15f + (sliderBar->isPressed() ? 0.1f : 0.0f));
	Draw::rectOrtho(Draw::defaultTexture, rect);
	Draw::alpha();

	Draw::color(Colors::GRAY);
	rect.setSize(sliderBar->getBarDrawSize());
	rect.move(sliderBar->getBarCurPos());
	Draw::rectOrtho(Draw::defaultTexture, rect);

	Draw::color(Colors::LIGHT_GRAY);
	rect.setSrc(sliderBar->getAbsSrc() + sliderBar->getBorder().bot_lft() + sliderBar->getBarLastPos());
	Draw::rectOrtho(Draw::defaultTexture, rect);
}

void UI::ProgressBarDrawer::draw(const ProgressBar* progressBar) const{
	Rect bound = progressBar->getValidBound().move(progressBar->getAbsSrc());

	Draw::color(Colors::DARK_GRAY);
	Draw::rectOrtho(Draw::contextTexture, bound);

	bound.sclSize(progressBar->getDrawProgress(), 1.0f);
	Draw::color(Colors::LIGHT_GRAY);
	Draw::rectOrtho(Draw::contextTexture, bound);
}

void UI::TextureRegionRectDrawable::draw(const Geom::OrthoRectFloat rect) const {
#ifdef _DEBUG
	if(!texRegion)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
	Draw::rectOrtho(texRegion, rect);
}

void UI::TextureNineRegionDrawable::draw(const Geom::OrthoRectFloat rect) const {
#ifdef _DEBUG
	if(!texRegion)throw ext::NullPointerException{"Null Tex On Draw Call!"};
#endif
	texRegion->render_RelativeExter(rect);
}

void UI::DrawPair::draw(const UI::Widget* elem, const float alphaScl, const Geom::OrthoRectFloat rect) const {
	Draw::color(color, alphaScl * color.a);
	region->draw(rect);
}

void UI::UIStyle::drawElem(const UI::Widget* elem) const {
	elem->tempColor = elem->color;
	elem->tempColor.a *= elem->maskOpacity;

	float alphaScl = elem->selfMaskOpacity * elem->maskOpacity;

	Draw::mixColor(elem->tempColor);
	const Rect rect = elem->getBound().setSrc(elem->getAbsSrc());
	base.draw(elem, alphaScl, rect);
	edge.draw(elem, alphaScl, rect);
	if(elem->isCursorInbound())inbound.draw(elem, alphaScl, rect);
	if(elem->isPressed())pressed.draw(elem, alphaScl, rect);

	Draw::mixColor();
	//TODO disabled
	// if(elem->touchDisabled())disabled.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
}

void UI::UIStyle::drawBackground(const Widget* elem) const{
	elem->tempColor = elem->color;
	elem->tempColor.a *= elem->maskOpacity;

	background.draw(elem, elem->selfMaskOpacity * elem->maskOpacity, elem->getBound().setSrc(elem->getAbsSrc()));
}

void UI::StyleDrawer::applyToElem(Widget* elem) {
	elem->setBorder(style->margin);

	elem->changed(ChangeSignal::notifyAll);
}

void UI::EdgeDrawer::drawStyle(const UI::Widget* elem) const {
	elem->tempColor = elem->color;
	Color& color = elem->tempColor;
	elem->tempColor.a *= elem->maskOpacity;
	Draw::mixColor(elem->tempColor);

	if(elem->isCursorInbound()) {
		color.mul(1.1f).lerp(Colors::WHITE, 0.3f);
		Draw::color(color);
		Draw::alpha(elem->isPressed() ? 0.5f : 0.2f);
		Draw::rectOrtho(Draw::defaultTexture, elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	}


	Draw::color(color);
	Draw::Line::setLineStroke(1.0f);
	Draw::alpha(color.a * elem->maskOpacity);
	Draw::Line::rectOrtho(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());

	Draw::reset();
}

void UI::EmptyDrawer::applyToElem(Widget* elem){
	elem->setBorder(0.0f);
}
