module;

export module Graphic.PostProcessor;

export import GL.Buffer.FrameBuffer;
export import GL.Texture.Texture2D;

using namespace GL;

export namespace Graphic {
	class PostProcessor {
	public:

		FrameBuffer* toProcess{nullptr};

		[[nodiscard]] explicit PostProcessor(FrameBuffer* const toProcess)
			: toProcess(toProcess) {
		}

		virtual void begin() = 0;

		virtual void process() = 0;

		virtual void end() = 0;

		virtual ~PostProcessor() = default;
	};
}
