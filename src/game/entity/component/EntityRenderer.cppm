module;

export module Game.Entity.Drawer;

import GL.Buffer.FrameBuffer;

export namespace Game {
	class EntityDrawer {
	protected:
		GL::FrameBuffer* context{nullptr};

	public:
		virtual ~EntityDrawer() = default;

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
