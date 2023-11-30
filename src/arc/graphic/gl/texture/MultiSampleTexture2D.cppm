module;

export module GL.Texture.MultiSampleTexture2D;

import <glad/glad.h>;

import GL.Texture.Texture2D;

export namespace GL{
	class MultiSampleTexture2D final :
			public Texture2D{
	public:
		int samples = 4;

		MultiSampleTexture2D(const unsigned int w, const unsigned int h, const int samples_) : Texture2D() {
			targetFlag = GL_TEXTURE_2D_MULTISAMPLE;

			glGenTextures(1, &textureID);

			glBindTexture(targetFlag, textureID);
			this->samples = samples_;
			glTexImage2DMultisample(targetFlag, samples_, GL_RGBA8, w, h, GL_TRUE);
			unbind();
		}

		MultiSampleTexture2D(const unsigned int w, const unsigned int h) : MultiSampleTexture2D(w, h, 4) {

		}

		void resize(const unsigned int w, const unsigned int h) override {
			width = w;
			height = h;

			bind();
			glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB, width, height, GL_TRUE);
			unbind();
		}
	};

}

