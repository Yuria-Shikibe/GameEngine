module;

export module GL.Texture.MultiSampleTexture2D;

import <glad/glad.h>;

import GL.Texture.Texture2D;

export namespace GL{
	class MultiSampleTexture2D final :
			public Texture2D{
	public:
		int samples = 4;

		MultiSampleTexture2D(const unsigned int w, const unsigned int h, const int samples) : Texture2D(), samples{samples} {
			width = w;
			height = h;
			targetFlag = GL_TEXTURE_2D_MULTISAMPLE;
			glCreateTextures(targetFlag, 1, &textureID);

			glTextureStorage2DMultisample(textureID, samples, GL_RGBA8, width, height, GL_TRUE);
		}

		MultiSampleTexture2D(const unsigned int w, const unsigned int h) : MultiSampleTexture2D(w, h, 4) {

		}

		void resize(const unsigned int w, const unsigned int h) override {
			if(width == w || height == h)return;
			width = w;
			height = h;

			glTextureStorage2DMultisample(textureID, samples, GL_RGBA8, width, height, GL_TRUE);
		}
	};

}

