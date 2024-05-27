//
// Created by Matrix on 2024/5/26.
//

export module Graphic.Draw:Frame;

export import std;
export import ext.Concepts;
export import GL.Mesh;
export import GL.Shader;
export import GL.Buffer.FrameBuffer;
export import GL.Constants;

export namespace Graphic::Frame{
	inline const GL::Mesh* rawMesh{ nullptr };
	inline const GL::ShaderProgram* blitter{ nullptr };

	template <Concepts::InvokeNullable<void(const GL::ShaderProgram&)> Func = std::nullptr_t>
	void blit(const GL::FrameBuffer* const draw, const unsigned port = 0, const GL::ShaderProgram* shader = blitter,
			  Func&& f = nullptr){
		draw->bind(GL::FrameBuffer::DRAW);
		draw->enableDrawAt(port);

		if(shader){
			shader->bind();
			shader->apply();
			if constexpr(!std::is_same_v<std::nullptr_t, Func>){
				shader->applyDynamic(f, false);
			}
		}

		rawMesh->bind();
		rawMesh->render(GL_TRIANGLE_FAN, 0, GL::ELEMENTS_QUAD_STRIP_LENGTH);
	}

	void blitCopyAll(const GL::FrameBuffer* const read, const GL::FrameBuffer* const draw,
					 const GLbitfield mask = GL_COLOR_BUFFER_BIT, const GLenum filter = GL_LINEAR){
		const unsigned int max = std::min(read->getColorAttachments().size(), draw->getColorAttachments().size());

		read->bind(GL::FrameBuffer::READ);
		draw->bind(GL::FrameBuffer::DRAW);

		for(unsigned i = 0; i < max; ++i){
			read->enableRead(i);
			draw->enableDrawAt(i);

			GL::blit(
				0, 0, read->getWidth(), read->getHeight(),
				0, 0, draw->getWidth(), draw->getHeight(),
				mask, filter);
		}
	}

	void blitCopy(const GL::FrameBuffer* const read, unsigned readAttachmentID, const GL::FrameBuffer* const draw,
				  unsigned drawAttachmentID,
				  const GLbitfield mask = GL_COLOR_BUFFER_BIT, const GLenum filter = GL_LINEAR){
		read->bind(GL::FrameBuffer::READ);
		draw->bind(GL::FrameBuffer::DRAW);

		read->enableRead(readAttachmentID);
		draw->enableDrawAt(drawAttachmentID);

		GL::blit(
			0, 0, read->getWidth(), read->getHeight(),
			0, 0, draw->getWidth(), draw->getHeight(),
			mask, filter);
	}
}
