module;

export module Game.Renderer;

import Core.Renderer;

export namespace Game {
	class Renderer : public Core::Renderer {
	public:
		[[nodiscard]] Renderer(const unsigned w, const unsigned h)
			: Core::Renderer(w, h) {
		}
	};
}
