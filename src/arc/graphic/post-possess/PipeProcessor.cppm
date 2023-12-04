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
			temp1.clear();
			temp2.clear();

			temp1.resize(toProcess->getWidth(), toProcess->getHeight());
			temp2.resize(toProcess->getWidth(), toProcess->getHeight());

			processors.front()->apply(toProcess, &temp1);
			flag = true;
		}

		void process() const override {
			for(size_t i = 1; i < processors.size() - 1; ++i) {
				FrameBuffer* read = flag ? &temp1 : &temp2;
				FrameBuffer* draw = flag ? &temp2 : &temp1;
				processors[i]->apply(read, draw);
				flag = !flag;

				read->clear();
			}
		}

		void end(FrameBuffer* target) const override {
			FrameBuffer* const read = flag ? &temp1 : &temp2;

			processors.back()->apply(read, target);
		}
	};
}