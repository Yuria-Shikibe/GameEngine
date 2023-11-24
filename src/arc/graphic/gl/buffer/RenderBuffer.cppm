module;

export module GL.Buffer.RenderBuffer;

import <glad/glad.h>;

export import GL.Buffer;
export import Graphic.Resizeable;

export namespace GL{
class RenderBuffer : virtual public GLBuffer, virtual public Graphic::ResizeableInt{
	protected:
		int width, height;
	public:
		RenderBuffer() : width(2), height(2){}

		RenderBuffer(const int w, const int h) : width(w), height(h){
			glGenRenderbuffers(1, &bufferID);
			targetFlag = GL_RENDERBUFFER;

			glBindRenderbuffer(targetFlag, bufferID);
			glRenderbufferStorage(targetFlag, GL_DEPTH24_STENCIL8, w, h);
			glBindRenderbuffer(targetFlag, 0);
		}

		~RenderBuffer() override{
			glDeleteRenderbuffers(1, &bufferID);
		}

		void bind() const override{
			glBindRenderbuffer(targetFlag, bufferID);
		}

		void unbind() const override{
			glBindRenderbuffer(targetFlag, 0);
		}

		void resize(const int w, const int h) override{
			width = w;
			height = h;
			bind();
			glRenderbufferStorage(targetFlag, GL_DEPTH24_STENCIL8, w, h);
			unbind();
		}
	};
}


