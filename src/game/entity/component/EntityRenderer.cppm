module;

export module Game.Entity.Renderer;

import GL.Buffer.FrameBuffer;

export namespace Game {
	class EntityRenderer {
	protected:
		GL::FrameBuffer* context{nullptr};

	public:
		virtual ~EntityRenderer() = default;

		[[nodiscard]] virtual GL::FrameBuffer* getContext() const {
			return context;
		}

		virtual void setContext(GL::FrameBuffer* const context) {
			this->context = context;
		}

		virtual void render() {

		}

		virtual void blitTo(GL::FrameBuffer* where) {

		}
	};

}
