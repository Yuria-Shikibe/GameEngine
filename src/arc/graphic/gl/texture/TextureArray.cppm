module;

#include <glad/glad.h>

export module GL.TextureArray;

export import GL.Texture.Texture2D;
export import GL.Texture;

import std;

export namespace GL{
	struct Texture2DArray : public GL::Texture{

		std::vector<const Texture2D*> layers{};

		Texture2DArray() : Texture(GL_TEXTURE_2D_ARRAY){}

		explicit Texture2DArray(const std::vector<const Texture2D*>& texture2Ds, const GLint mipmapLevel = Texture2D::MipMapGeneralLevel) : Texture(GL_TEXTURE_2D_ARRAY){
			init(texture2Ds, mipmapLevel);
		}

		void init(const std::vector<const Texture2D*>& texture2Ds, const GLint mipmapLevel = Texture2D::MipMapGeneralLevel){
			this->layers = texture2Ds;

			const Texture2D* standard = layers.front();
			width = standard->getWidth();
			height = standard->getHeight();

			glCreateTextures(targetFlag, 1, &nameID);
			glTextureStorage3D(nameID, mipmapLevel, GL_RGBA8, width, height, layers.size());

			for(int i = 0; i < layers.size(); ++i){
				const Texture2D* tex = layers.at(i);
				glCopyImageSubData(
					tex->getID(), tex->getTargetFlag(), 0, 0, 0, 0,
					nameID, targetFlag, 0, 0, 0, i,
					tex->getWidth(), tex->getHeight(), 1);
			}

			glGenerateTextureMipmap(nameID);
		}

		void resize(unsigned w, unsigned h) override{

		}

		void active(const unsigned offset) const override{
			glBindTextureUnit(offset, nameID);
		}
		void activeAll(const unsigned offset) const override{
			glBindTextureUnit(offset, nameID);
		}

		~Texture2DArray() override{
			if(nameID)glDeleteTextures(1, &nameID);
		}
	};
}
