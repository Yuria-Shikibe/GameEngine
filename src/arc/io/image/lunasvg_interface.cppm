export module Image.Svg;

import Graphic.Pixmap;
import OS.File;

export namespace ext{
	[[nodiscard]] Graphic::Pixmap svgToBitmap(const OS::File& file, const unsigned int width = 0, const unsigned int height = 0);
}