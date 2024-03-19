module;

export module Graphic.PostProcessor.MultiSampleBliter;

import Graphic.PostProcessor;
import Graphic.Draw;
import GL.Constants;

using GL::FrameBuffer;

import std;

export namespace Graphic {
	template <size_t size = 0, std::array<std::pair<unsigned, unsigned>, size> idArray = {}, GLbitfield mask = GL_COLOR_BUFFER_BIT, GL::TexParams filter = GL::TexParams::linear>
	class MultiSampleBliter final : public PostProcessor {
	public:
		static constexpr auto BlitMask = mask;
		static constexpr auto BiltFilter = filter;

		[[nodiscard]] MultiSampleBliter() = default;

		[[nodiscard]] explicit MultiSampleBliter(FrameBuffer* const toProcess)
			: PostProcessor(toProcess) {
		}

		void beginProcess() const override {}

		void endProcess(FrameBuffer* target) const override {
			if(target == nullptr || toProcess == nullptr)throwException();

			if constexpr (size == 0){

				Draw::blitCopyAll(toProcess, target);
			}else{

				for(const auto [readID, drawID] : idArray){
					Draw::blitCopy(toProcess, readID, target, drawID);
				}
			}
		}

		void runProcess() const override {}
	};
}
