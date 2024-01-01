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

void GL::Texture2D::resize(const unsigned w, const unsigned h) {
	if(w == width && h == height)return;

	free();
	width  = w;
	height = h;

	bind();
	glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
	unbind();
}

unsigned char* GL::Texture2D::loadFromFile(const OS::File& file) {
	//TODO File Support
	return stbi::loadPng(file, width, height, bpp);
}

void GL::Texture2D::init(unsigned char*&& data) {
	if(data != localData.get())localData.reset(data);

	if(bpp != 4) {
		throw ext::RuntimeException{"Illegal Bpp Size: " + bpp};
	}

	glGenTextures(1, &textureID);

	bind();

	//TODO : Check if needed here.
	glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
	setParameters();
	bind();
	glGenerateMipmap(targetFlag);
	unbind();

	free();
}

void GL::Texture2D::updateData() { // NOLINT(*-make-member-function-const)
	bind();

	//Avoid unnecessary resize allocate
	if(!valid()) {
		localData.reset(new unsigned char[width * height * 4]);
	}

	glGetTexImage(targetFlag, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
	unbind();
}

void GL::Texture2D::setParameters(const GLint downScale, const GLint upScale, const GLint clampX,
	const GLint clampY) const {
	bind();
	glTexParameteri(targetFlag, GL_TEXTURE_MIN_FILTER, downScale);
	glTexParameteri(targetFlag, GL_TEXTURE_MAG_FILTER, upScale);
	glTexParameteri(targetFlag, GL_TEXTURE_WRAP_S, clampX);
	glTexParameteri(targetFlag, GL_TEXTURE_WRAP_T, clampY);
	unbind();
}

void GL::Texture2D::setScale(const GLint downScale, const GLint upScale) const {
	bind();
	glTexParameteri(targetFlag, GL_TEXTURE_MIN_FILTER, downScale);
	glTexParameteri(targetFlag, GL_TEXTURE_MAG_FILTER, upScale);
	unbind();
}

void GL::Texture2D::setWrap(const GLint clamp) const {
	bind();
	glTexParameteri(targetFlag, GL_TEXTURE_WRAP_S, clamp);
	glTexParameteri(targetFlag, GL_TEXTURE_WRAP_T, clamp);
	unbind();
}

std::unique_ptr<unsigned char[]> GL::Texture2D::copyData() const {
	if(!valid()) {
		throw ext::RuntimeException{"Texture doesn't contains data!"};
	}

	const size_t size = dataSize();
	auto* data        = new unsigned char[size]{0};

	std::copy_n(localData.get(), size, data);

	return std::unique_ptr<unsigned char[]>(data);
}
