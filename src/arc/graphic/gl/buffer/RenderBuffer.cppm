module;

export module GL.Buffer.RenderBuffer;

import <glad/glad.h>;
export import GL.Object;
export import Graphic.Resizeable;

export namespace GL{
class RenderBuffer : public GLObject, public Graphic::ResizeableUInt{
	protected:
		unsigned int width, height;
	public:
		RenderBuffer() : GLObject{GL_RENDERBUFFER}, width(2), height(2){}

		RenderBuffer(const unsigned int w, const unsigned int h) : GLObject{GL_RENDERBUFFER}, width(w), height(h){
			glCreateRenderbuffers(1, &nameID);
			glNamedRenderbufferStorage(nameID, GL_DEPTH32F_STENCIL8, w, h);
		}

		~RenderBuffer() override {
			if(nameID)glDeleteRenderbuffers(1, &nameID);
		}

		void bind() const{
			glBindRenderbuffer(targetFlag, nameID);
		}

		void unbind() const{
			glBindRenderbuffer(targetFlag, 0);
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;
			glNamedRenderbufferStorage(nameID, GL_DEPTH32F_STENCIL8, w, h);
		}
	};
}


