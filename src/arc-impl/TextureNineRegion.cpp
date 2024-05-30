module GL.Texture.TextureNineRegion;

import Graphic.Draw;

using namespace Graphic;

void GL::TextureNineRegion::render_RelativeExter(const float x, const float y, const float width, const float height) const {
	//TODO clamp
	Align::Spacing tempEdge = edge;
	if(width < edge.getWidth()){
		const float remain = edge.getWidth() - width;
		tempEdge.left -= remain * 0.5f;
		tempEdge.right -= remain * 0.5f;
	}

	if(height < edge.getHeight()){
		const float remain = edge.getHeight() - height;
		tempEdge.bottom -= remain * 0.5f;
		tempEdge.top -= remain * 0.5f;
	}
	
	Draw::Overlay::Fill::rectOrtho(regions[ID_center], x + tempEdge.left, y + tempEdge.bottom, width - tempEdge.getWidth(), height - tempEdge.getHeight());

	Draw::Overlay::Fill::rectOrtho(regions[ID_right], x + width - tempEdge.right, y + tempEdge.bottom, tempEdge.right, height - tempEdge.bottom - tempEdge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_top], x + tempEdge.left, y + height - tempEdge.top, width - tempEdge.left - tempEdge.right, tempEdge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_left], x, y + tempEdge.bottom, tempEdge.left,  height - tempEdge.bottom - tempEdge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottom], x + tempEdge.left, y, width - tempEdge.left - tempEdge.right,  tempEdge.bottom);

	Draw::Overlay::Fill::rectOrtho(regions[ID_topRight], x + width - tempEdge.right, y + height - tempEdge.top, tempEdge.right, tempEdge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_topLeft], x, y + height - tempEdge.top, tempEdge.left, tempEdge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomLeft], x, y, tempEdge.left, tempEdge.bottom);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomRight], x + width - tempEdge.right, y, tempEdge.right, tempEdge.bottom);
}

void GL::TextureNineRegion::render_RelativeInner(const float x, const float y, const float width,
		const float height) const {
	Draw::Overlay::Fill::rectOrtho(regions[ID_center], x, y, width, height);

	Draw::Overlay::Fill::rectOrtho(regions[ID_right], x + width, y, edge.right, height);
	Draw::Overlay::Fill::rectOrtho(regions[ID_top], x, y + height, width, edge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_left], x - edge.left, y, edge.left, height);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottom], x, y - edge.bottom, width, edge.bottom);

	Draw::Overlay::Fill::rectOrtho(regions[ID_topRight], x + width, y + height, edge.right, edge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_topLeft], x - edge.left, y + height, edge.left, edge.top);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomLeft], x - edge.left, y - edge.bottom, edge.left, edge.bottom);
	Draw::Overlay::Fill::rectOrtho(regions[ID_bottomRight], x + width, y - edge.bottom, edge.right, edge.bottom);
}