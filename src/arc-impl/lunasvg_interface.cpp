module;

#define THIRD_PARTY_USING_IMPORT
#include <../include/lunasvg/lunasvg/include/lunasvg.h>

module Image.Svg;

Graphic::Pixmap ext::svgToBitmap(const OS::File& file, const unsigned int width, const unsigned int height){
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

	for(int i = 0; i < pixmap.pixelSize(); ++i){
		std::swap(pixmap[i * 4 + 0], pixmap[i * 4 + 2]);
	}

	return pixmap;
}