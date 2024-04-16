module;

#define THIRD_PARTY_USING_IMPORT
#include <../include/lunasvg/lunasvg/include/lunasvg.h>

export module Image.Svg;

import Graphic.Pixmap;
import OS.File;

export namespace ext{
	[[nodiscard]] Graphic::Pixmap svgToBitmap(const OS::File& file, const unsigned int width = 0, const unsigned int height = 0) {
		const auto document = lunasvg::Document::loadFromFile(file.getPath().string());

		lunasvg::Bitmap bitmap;

		if(width > 0){
			if(height > 0){
				bitmap = document->renderToBitmap(width, height);
			}else{
				bitmap = document->renderToBitmap(width);
			}
		}else{
			bitmap = document->renderToBitmap();
		}

		Graphic::Pixmap pixmap{static_cast<int>(bitmap.width()), static_cast<int>(bitmap.height())};

		pixmap.copyFrom(bitmap.data(), true);

		return pixmap;
	}
}