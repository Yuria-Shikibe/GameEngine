module UI.Drawer;

import UI.Elem;
import Graphic.Color;
import Graphic.Draw;
import ext.RuntimeException;
import UI.ProgressBar;
import UI.SliderBar;

using namespace Graphic;
using Draw::Overlay;

void UI::SliderBarDrawer::draw(const SliderBar* sliderBar) const{
	//TODO trans these into the slide bar drawer
	Rect rect = sliderBar->getValidBound().move(sliderBar->getAbsSrc());

	Overlay::color(Colors::LIGHT_GRAY);
	Overlay::alpha(0.15f + (sliderBar->isPressed() ? 0.1f : 0.0f));
	Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), rect);
	Overlay::alpha();

	Overlay::color(Colors::GRAY);
	rect.setSize(sliderBar->getBarDrawSize());
	rect.move(sliderBar->getBarCurPos());
	Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), rect);

	Overlay::color(Colors::LIGHT_GRAY);
	rect.setSrc(sliderBar->getAbsSrc() + sliderBar->getBorder().bot_lft() + sliderBar->getBarLastPos());
	Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), rect);
}

void UI::ProgressBarDrawer::draw(const ProgressBar* progressBar) const{
	Rect bound = progressBar->getValidBound().move(progressBar->getAbsSrc());

	Overlay::color(Colors::DARK_GRAY);
	Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), bound);

	bound.sclSize(progressBar->getDrawProgress(), 1.0f);
	Overlay::color(Colors::LIGHT_GRAY);
	Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), bound);
}

void UI::TextureRegionRectDrawable::draw(const Geom::OrthoRectFloat rect) const {
#ifdef _DEBUG
	if(!texRegion)throw ext::NullPointerException{"Null Tex On Overlay Call!"};
#endif
	Overlay::Fill::rectOrtho(*texRegion, rect);
}

void UI::DrawPair::draw(const UI::Elem* elem, const float alphaScl, const Geom::OrthoRectFloat rect) const {
	if(!region)return;
	Overlay::color(color, alphaScl * color.a);
	region->draw(rect);
}

void UI::UIStyle::drawElem(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	elem->tempColor.a *= elem->maskOpacity;

	float alphaScl = elem->selfMaskOpacity * elem->maskOpacity;

	Overlay::mixColor(elem->tempColor);
	const Rect rect = elem->getBound().setSrc(elem->getAbsSrc());
	base.draw(elem, alphaScl, rect);
	edge.draw(elem, alphaScl, rect);

	if(elem->isCursorInbound())inbound.draw(elem, alphaScl, rect);
	if(elem->isActivated()){

		activated.draw(elem, alphaScl, rect);
	}
	if(elem->isPressed())pressed.draw(elem, alphaScl, rect);

	//TODO disabled
	// if(elem->touchDisabled())disabled.draw(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
}

void UI::UIStyle::drawBackground(const UI::Elem* elem) const{
	elem->tempColor = elem->color;
	elem->tempColor.a *= elem->maskOpacity;

	baseMask.draw(elem, elem->selfMaskOpacity * elem->maskOpacity, elem->getBound().setSrc(elem->getAbsSrc()));
}

void UI::UIStyle::drawBackground(const Geom::OrthoRectFloat rect) const{
	baseMask.draw(nullptr, 1, rect);
}

void UI::StyleDrawer::applyToElem(UI::Elem* elem) {
	elem->setBorder(style->margin);

	elem->changed(ChangeSignal::notifyAll);
}

void UI::EdgeDrawer::drawStyle(const UI::Elem* elem) const {
	elem->tempColor = elem->color;
	Color& color = elem->tempColor;
	elem->tempColor.a *= elem->maskOpacity;
	Overlay::mixColor(elem->tempColor);

	if(elem->isCursorInbound()) {
		color.mul(1.1f).lerp(Colors::WHITE, 0.3f);
		Overlay::color(color);
		Overlay::alpha(elem->isPressed() ? 0.5f : 0.2f);
		Overlay::Fill::rectOrtho(Overlay::getDefaultTexture(), elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());
	}


	Overlay::color(color);
	Overlay::Line::setLineStroke(1.0f);
	Overlay::alpha(color.a * elem->maskOpacity);
	Overlay::Line::rectOrtho(elem->drawSrcX(), elem->drawSrcY(), elem->getWidth(), elem->getHeight());

	Overlay::reset();
}

void UI::EmptyDrawer::applyToElem(UI::Elem* elem){
	elem->setBorder(0.0f);
}
