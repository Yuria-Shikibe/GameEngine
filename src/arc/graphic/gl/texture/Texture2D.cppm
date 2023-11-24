module;

#include <glad/glad.h>
#include "../src/arc/io/Image.h"

export module GL.Texture.Texture2D;

import File;
import Graphic.Resizeable;
import <string>;
import <ostream>;

export namespace GL{
	class Texture2D : virtual public Graphic::ResizeableInt
	{

	protected:
		unsigned char* localData = nullptr;
		GLuint textureID = 0;

	public:
		std::string texName;
		GLenum targetFlag = GL_TEXTURE_2D;
		int width = 0, height = 0;
		int bpp = 0;

		Texture2D() = default;

		~Texture2D() override{
			glDeleteTextures(1, &textureID);
		}

		Texture2D(const int w, const int h): width(w), height(h){
			glGenTextures(1, &textureID);
			bind();

			glTexImage2D(targetFlag, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glGenerateMipmap(targetFlag);
			setParametersDef();
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

		Texture2D(const Texture2D& other): texName(other.texName),
		                                   textureID(other.textureID),
		                                   width(other.width),
		                                   height(other.height),
		                                   bpp(other.bpp){
		}

		Texture2D(Texture2D&& other) noexcept: localData(other.localData),
		                                       texName(std::move(other.texName)),
		                                       textureID(other.textureID),
		                                       width(other.width),
		                                       height(other.height),
		                                       bpp(other.bpp){
		}

		Texture2D& operator=(const Texture2D& other){
			if(this == &other) return *this;
			texName = other.texName;
			textureID = other.textureID;
			width = other.width;
			height = other.height;
			bpp = other.bpp;
			return *this;
		}

		Texture2D& operator=(Texture2D&& other) noexcept{
			if(this == &other) return *this;
			texName = std::move(other.texName);
			textureID = other.textureID;
			width = other.width;
			height = other.height;
			bpp = other.bpp;
			return *this;
		}

		void resize(const int w, const int h) override{
			bind();
			glTexImage2D(targetFlag, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData);
			width = w;
			height = h;
			unbind();
		}

		void loadFromFile(const OS::File& file){
			stbi_set_flip_vertically_on_load(1);

			//TODO File Support
			localData = stbi_load(file.absolutePath().string().data(), &width, &height, &bpp, 4);

			glGenTextures(1, &textureID);

			setParametersDef();

			active();
			bind();
			 //TODO : Check if needed here.
			glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData);
			glGenerateMipmap(targetFlag);
			unbind();

			if (localData) {
				stbi_image_free(localData);
				localData = nullptr;
			}
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
		
		void active(const unsigned char slotOffset = 0) const{
			glActiveTexture(GL_TEXTURE0 + slotOffset);
		}

		void bindParam(const GLenum target) const{
			glBindTexture(target, textureID);
		}

		void unbind() const{
			glBindTexture(targetFlag, 0);
		}

		[[nodiscard]] GLuint getID() const{
			return textureID;
		}
	};
}