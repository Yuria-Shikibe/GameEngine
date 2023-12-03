module;

export module Graphic.PostProcessor.FilterProcessor;

import <glad/glad.h>;
import <functional>;
import Graphic.PostProcessor;
import Graphic.Draw;
import GL.Shader;

using GL::FrameBuffer;

export namespace Graphic {
	class FilterProcessor : public PostProcessor{
	public:
		[[nodiscard]] explicit FilterProcessor(FrameBuffer* const toProcess)
			: PostProcessor(toProcess) {
		}

		[[nodiscard]] FilterProcessor(FrameBuffer* const toProcess, GL::Shader* const shader)
			: PostProcessor(toProcess),
			shader(shader) {
		}

		[[nodiscard]] explicit FilterProcessor(GL::Shader* const shader)
			: shader(shader) {
		}

		[[nodiscard]] FilterProcessor(GL::Shader* const shader,
			const std::function<void(const GL::Shader&)>& shaderHandler)
			: shader(shader),
			shaderHandler(shaderHandler) {
		}

		GL::Shader* shader{nullptr};
		std::function<void(const GL::Shader&)> shaderHandler{nullptr};

		void begin() override {

		}

		void end(FrameBuffer* target) override {
			if(shader == nullptr || toProcess == nullptr || target == nullptr)throwException();
			// toProcess->bind(FrameBuffer::READ);

			toProcess->getTexture().active();
			toProcess->getTexture().bind();


			target->bind(FrameBuffer::DEF);

			Draw::blit(shader, shaderHandler);
		}

		void process() override {

		}
	};
}
