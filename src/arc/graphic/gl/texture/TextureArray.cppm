module;

#include <glad/glad.h>

export module GL.TextureArray;

export import GL.Texture.Texture2D;

import <memory>;
import <span>;
import <vector>;

export namespace GL{
	struct Texture2DArray {
		GLsizei width{};
		GLsizei height{};
		std::vector<const Texture2D*> layers{};

		GLuint arrayID{0};
		static constexpr GLenum targetFlag = GL_TEXTURE_2D_ARRAY;

		explicit Texture2DArray(const std::vector<const Texture2D*>& texture2Ds, const GLint mipmapLevel = Texture2D::MipMapGeneralLevel) : layers(texture2Ds){
			const Texture2D* standard = layers.front();
			width = standard->getWidth();
			height = standard->getHeight();

			glCreateTextures(targetFlag, 1, &arrayID);
			glTextureStorage3D(arrayID, mipmapLevel, GL_RGBA8, width, height, layers.size());

			for(int i = 0; i < layers.size(); ++i){
				const Texture2D* tex = layers.at(i);
				glCopyImageSubData(
					tex->getID(), tex->getTargetFlag(), 0, 0, 0, 0,
					arrayID, targetFlag, 0, 0, 0, i,
					tex->getWidth(), tex->getHeight(), 1);
			}

			glGenerateTextureMipmap(arrayID);
		}

		~Texture2DArray(){
			if(arrayID)glDeleteTextures(1, &arrayID);
		}
	};
}
