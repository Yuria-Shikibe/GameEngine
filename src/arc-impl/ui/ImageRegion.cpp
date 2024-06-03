module UI.ImageRegion;

import Graphic.Draw;

void UI::ImageRegion::drawContent() const{
	if(drawable){
		tempColor = color;
		tempColor.a *= selfMaskOpacity * maskOpacity * (isDisabled() ? 0.85f : 1.f);
		if(isDisabled())tempColor.mul(0.8f);

		Graphic::Draw::Overlay::color(tempColor);

		const auto size = Align::embedTo(scaling, drawable->getDefSize(), getValidSize());
		const auto offset = Align::getOffsetOf(imageAlign, size, getValidBound());

		drawable->draw(Geom::OrthoRectFloat{size.x, size.y}.setSrc(offset + absoluteSrc));
	}
}
