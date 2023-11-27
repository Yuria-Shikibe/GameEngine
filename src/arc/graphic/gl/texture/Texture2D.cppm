module;

export module GL.Texture.Texture2D;

import File;
import Graphic.Resizeable;
import <string>;
import <ostream>;
import <memory>;
import <glad/glad.h>;
import Image;

export namespace GL{
	class Texture2D : virtual public Graphic::ResizeableInt
	{

	protected:
		GLuint textureID = 0;

	public:
		struct STBI_DataDeleter {
			void operator()(unsigned char* t) const {
				if(t) {
					stbi::free(t);
				}
			}
		};

		std::string texName;
		GLenum targetFlag = GL_TEXTURE_2D;
		unsigned int width = 0, height = 0;
		unsigned int bpp{4};

		std::unique_ptr<unsigned char, STBI_DataDeleter> localData = nullptr;

		Texture2D() = default;

		~Texture2D() override{
			glDeleteTextures(1, &textureID);
		}

		Texture2D(const unsigned int w, const unsigned int h, unsigned char* data): width(w), height(h){
			localData.reset(data);

			glGenTextures(1, &textureID);
			bind();

			if(!data)localData.reset(new unsigned char[w * h * 4]);
			glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());

			glGenerateMipmap(targetFlag);
			setParametersDef();
			free();
		}

		Texture2D(const unsigned int w, const unsigned int h, std::unique_ptr<unsigned char[]>&& data) : Texture2D(w, h, data.release()) {

		}

		Texture2D(const unsigned int w, const unsigned int h) : Texture2D(w, h, nullptr) {

		}

		explicit Texture2D(const OS::File& file){
			loadFromFile(file);
			texName = file.stem();
		}

		static std::size_t hash_value(const Texture2D& obj){
			return obj.textureID;
		}

		friend std::ostream& operator<<(std::ostream& os, const Texture2D& obj){
			return os << "Texture: " << obj.texName;
		}

		friend bool operator==(const Texture2D& lhs, const Texture2D& rhs){
			return lhs.texName == rhs.texName;
		}

		friend bool operator!=(const Texture2D& lhs, const Texture2D& rhs){
			return !(lhs == rhs);
		}

		Texture2D(const Texture2D& other) noexcept: localData(other.copyData().release()),
											   texName(other.texName),
											   textureID(other.textureID),
		                                       targetFlag(other.targetFlag),
											   width(other.width),
											   height(other.height),
											   bpp(other.bpp){
		}

		Texture2D(Texture2D&& other) noexcept: localData(std::move(other.localData)),
		                                       texName(std::move(other.texName)),
		                                       textureID(other.textureID),
											   targetFlag(other.targetFlag),
		                                       width(other.width),
		                                       height(other.height),
		                                       bpp(other.bpp){
		}

		Texture2D& operator=(Texture2D&& other) noexcept{
			if(this == &other) return *this;
			localData = std::move(other.localData);
			texName = std::move(other.texName);
			textureID = other.textureID;
			width = other.width;
			height = other.height;
			bpp = other.bpp;
			return *this;
		}

		void free() {
			localData.reset(nullptr);
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if(w == width && h == height)return;

			free();

			bind();
			glTexImage2D(targetFlag, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
			width = w;
			height = h;
			unbind();
		}

		[[nodiscard]] bool valid() const {
			return localData != nullptr;
		}

		void loadFromFile(const OS::File& file){
			//TODO File Support
			unsigned char* data = stbi::loadPng(file, width, height, bpp);
			localData.reset(data);

			glGenTextures(1, &textureID);

			setParametersDef();

			active();
			bind();
			 //TODO : Check if needed here.
			glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
			glGenerateMipmap(targetFlag);
			unbind();

			free();
		}

		void updateData() {
			bind();

			//Avoid unnecessary resize allocate
			if(!valid()) {
				localData.reset(new unsigned char[width * height * 4]);
			}

			glGetTexImage(targetFlag, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
		}

		void setParametersDef() const{
			bind();
			glTexParameteri(targetFlag, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(targetFlag, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			unbind();
		}

		void setScale(const GLint upScale, const GLint downScale) const{
			bind();
			glTexParameteri(targetFlag, GL_TEXTURE_MIN_FILTER, downScale);
			glTexParameteri(targetFlag, GL_TEXTURE_MAG_FILTER, upScale);
			unbind();
		}

		void setWrap(const GLint clamp) const{
			bind();
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_S, clamp);
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_T, clamp);
			unbind();
		}

		void bind() const{
			glBindTexture(targetFlag, textureID);
		}
		
		void active(const unsigned char slotOffset = 0) const{ // NOLINT(*-convert-member-functions-to-static)
			glActiveTexture(GL_TEXTURE0 + slotOffset);
		}

		void bindParam(const GLenum target) const{
			glBindTexture(target, textureID);
		}

		void unbind() const{
			glBindTexture(targetFlag, 0);
		}

		[[nodiscard]] size_t dataSize() const { //How many bytes!
			if(!localData)return 0;

			return width * height * bpp;
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> copyData() const {
			const size_t size = dataSize();
			auto* data = new unsigned char[size]{0};
			std::copy_n(localData.get(), size, data);

			return std::unique_ptr<unsigned char[]>(data);
		}

		[[nodiscard]] GLuint getID() const{
			return textureID;
		}

		void setID(const GLuint id){
			textureID = id;;
		}
	};
}