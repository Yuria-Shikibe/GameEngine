//
// Created by Matrix on 2023/12/3.
//

module;

export module Graphic.PostProcessor.MultiSampleBliter;

import <glad/glad.h>;
import Graphic.PostProcessor;
import Graphic.Draw;

using GL::FrameBuffer;

export namespace Graphic {
	class MultiSampleBliter final : public PostProcessor {
	public:
		[[nodiscard]] MultiSampleBliter() = default;

		[[nodiscard]] explicit MultiSampleBliter(FrameBuffer* const toProcess)
			: PostProcessor(toProcess) {
		}

		void begin() const override {

		}

		void end(FrameBuffer* target) const override {
			if(target == nullptr || toProcess == nullptr)throwException();

			Draw::blitRaw(toProcess, target);

		}

		void process() const override {}

	};
}
