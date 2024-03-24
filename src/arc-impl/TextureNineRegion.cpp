module GL.Texture.TextureNineRegion;

import Graphic.Draw;

using namespace Graphic;

void GL::TextureNineRegion::render_RelativeExter(const float x, const float y, const float width, const float height) const {
	if(regions[ID_center     ].getData())Draw::rectOrtho(&regions[ID_center], x + bottomLeftSize.x, y + bottomLeftSize.y, width - bottomLeftSize.x - topRightSize.x, height - bottomLeftSize.y - topRightSize.y);

	if(regions[ID_right      ].getData())Draw::rectOrtho(&regions[ID_right], x + width - topRightSize.x, y + bottomLeftSize.y, topRightSize.x, height - bottomLeftSize.y - topRightSize.y);
	if(regions[ID_top        ].getData())Draw::rectOrtho(&regions[ID_top], x + bottomLeftSize.x, y + height - topRightSize.y, width - bottomLeftSize.x - topRightSize.x, topRightSize.y);
	if(regions[ID_left       ].getData())Draw::rectOrtho(&regions[ID_left], x, y + bottomLeftSize.y, bottomLeftSize.x,  height - bottomLeftSize.y - topRightSize.y);
	if(regions[ID_bottom     ].getData())Draw::rectOrtho(&regions[ID_bottom], x + bottomLeftSize.x, y, width - bottomLeftSize.x - topRightSize.x,  bottomLeftSize.y);

	if(regions[ID_topRight   ].getData())Draw::rectOrtho(&regions[ID_topRight], x + width - topRightSize.x, y + height - topRightSize.y, topRightSize.x, topRightSize.y);
	if(regions[ID_topLeft    ].getData())Draw::rectOrtho(&regions[ID_topLeft], x, y + height - topRightSize.y, bottomLeftSize.x, topRightSize.y);
	if(regions[ID_bottomLeft ].getData())Draw::rectOrtho(&regions[ID_bottomLeft], x, y, bottomLeftSize.x, bottomLeftSize.y);
	if(regions[ID_bottomRight].getData())Draw::rectOrtho(&regions[ID_bottomRight], x + width - topRightSize.x, y, topRightSize.x, bottomLeftSize.y);
}

void GL::TextureNineRegion::render_RelativeInner(const float x, const float y, const float width,
		const float height) const {
	if(regions[ID_center     ].getData())Draw::rectOrtho(&regions[ID_center], x, y, width, height);

	if(regions[ID_right      ].getData())Draw::rectOrtho(&regions[ID_right], x + width, y, topRightSize.x, height);
	if(regions[ID_top        ].getData())Draw::rectOrtho(&regions[ID_top], x, y + height, width, topRightSize.y);
	if(regions[ID_left       ].getData())Draw::rectOrtho(&regions[ID_left], x - bottomLeftSize.x, y, bottomLeftSize.x, height);
	if(regions[ID_bottom     ].getData())Draw::rectOrtho(&regions[ID_bottom], x, y - bottomLeftSize.y, width, bottomLeftSize.y);

	if(regions[ID_topRight   ].getData())Draw::rectOrtho(&regions[ID_topRight], x + width, y + height, topRightSize.x, topRightSize.y);
	if(regions[ID_topLeft    ].getData())Draw::rectOrtho(&regions[ID_topLeft], x - bottomLeftSize.x, y + height, bottomLeftSize.x, topRightSize.y);
	if(regions[ID_bottomLeft ].getData())Draw::rectOrtho(&regions[ID_bottomLeft], x - bottomLeftSize.x, y - bottomLeftSize.y, bottomLeftSize.x, bottomLeftSize.y);
	if(regions[ID_bottomRight].getData())Draw::rectOrtho(&regions[ID_bottomRight], x + width, y - bottomLeftSize.y, topRightSize.x, bottomLeftSize.y);
}