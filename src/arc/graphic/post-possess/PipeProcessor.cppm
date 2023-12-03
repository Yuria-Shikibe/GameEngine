//
// Created by Matrix on 2023/12/3.
//

module;

export module Graphic.PostProcessor.PipeProcessor;

export import Graphic.PostProcessor;
import Graphic.Draw;
import <vector>;

using namespace GL;

export namespace Graphic {
	class PipeProcessor final : public PostProcessor {
		std::vector<PostProcessor*> processors{};

		mutable FrameBuffer temp1{2, 2};
		mutable FrameBuffer temp2{2, 2};

		mutable bool flag = false;
	public:
		[[nodiscard]] PipeProcessor() = default;

		[[nodiscard]] explicit PipeProcessor(const std::initializer_list<PostProcessor*> processors) : processors(processors) {

		}

		void add(PostProcessor* processor) {
			processors.push_back(processor);
		}

		PipeProcessor& operator<<(PostProcessor* processor) {
			processors.push_back(processor);
			return *this;
		}

		void begin() const override {
			temp1.resize(toProcess->getWidth(), toProcess->getHeight());
			temp2.resize(toProcess->getWidth(), toProcess->getHeight());

			Draw::blitRaw(toProcess, &temp1);
			flag = false;
		}

		void process() const override {
			for(const auto& processor : processors) {
				FrameBuffer* const read = flag ? &temp1 : &temp2;
				FrameBuffer* const draw = flag ? &temp2 : &temp1;
				processor->apply(read, draw);
				flag = !flag;

				read->clear();
			}
		}

		void end(FrameBuffer* target) const override {
			const FrameBuffer* const read = flag ? &temp1 : &temp2;
			Draw::blitRaw(read, target);
		}
	};
}