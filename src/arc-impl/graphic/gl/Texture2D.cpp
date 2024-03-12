module;

#include <glad/glad.h>

module GL.Texture.Texture2D;

GL::Texture2D::Texture2D(const OS::File& file, const bool initInstantly) {
	localData.reset(loadFromFile(file));
	if(initInstantly)init();
	texName = file.stem();
}

GL::Texture2D::Texture2D(const Texture2D& other) noexcept: textureID(other.textureID),
	targetFlag(other.targetFlag),
	width(other.width),
	height(other.height),
	bpp(other.bpp),
	texName(other.texName),
	localData(other.copyData().release()) {
}

GL::Texture2D::Texture2D(Texture2D&& other) noexcept: textureID(other.textureID),
	targetFlag(other.targetFlag),
	width(other.width),
	height(other.height),
	bpp(other.bpp),
	texName(std::move(other.texName)),
	localData(std::move(other.localData)) {
	textureID = 0;
}

GL::Texture2D& GL::Texture2D::operator=(Texture2D&& other) noexcept {
	if(this == &other) return *this;
	localData = std::move(other.localData);
	texName   = std::move(other.texName);
	textureID = other.textureID;
	width     = other.width;
	height    = other.height;
	bpp       = other.bpp;

	other.textureID = 0;
	return *this;
}