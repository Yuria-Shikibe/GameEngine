module;

export module Graphic.PostProcessor.ShaderProcessor;

import std;
import Graphic.PostProcessor;
import Graphic.Draw;
import GL.Shader;

using GL::FrameBuffer;

export namespace Graphic {
	class ShaderProcessor final : public PostProcessor{
	public:
		[[nodiscard]] explicit ShaderProcessor(FrameBuffer* const toProcess)
			: PostProcessor(toProcess) {
		}

		[[nodiscard]] ShaderProcessor(FrameBuffer* const toProcess, const GL::Shader* const shader)
			: PostProcessor(toProcess),
			shader(shader) {
		}

		[[nodiscard]] explicit ShaderProcessor(const GL::Shader* const shader)
			: shader(shader) {
		}

		[[nodiscard]] ShaderProcessor() : ShaderProcessor(Draw::blitter) { // NOLINT(*-use-equals-default)

		}

		[[nodiscard]] ShaderProcessor(const GL::Shader* const shader,
		                              const std::function<void(const GL::Shader&)>& shaderHandler)
			: shader(shader),
			shaderHandler(shaderHandler) {
		}

		ShaderProcessor(const ShaderProcessor& other)
			: PostProcessor(other),
			shader(other.shader),
			shaderHandler(other.shaderHandler) {
		}

		ShaderProcessor(ShaderProcessor&& other) noexcept
			: PostProcessor(std::move(other)),
			shader(other.shader),
			shaderHandler(std::move(other.shaderHandler)) {
		}

		ShaderProcessor& operator=(const ShaderProcessor& other) {
			if(this == &other) return *this;
			PostProcessor::operator =(other);
			shader = other.shader;
			shaderHandler = other.shaderHandler;
			return *this;
		}

		ShaderProcessor& operator=(ShaderProcessor&& other) noexcept {
			if(this == &other) return *this;
			PostProcessor::operator =(std::move(other));
			shader = other.shader;
			shaderHandler = std::move(other.shaderHandler);
			return *this;
		}

		const GL::Shader* shader{nullptr};
		std::function<void(const GL::Shader&)> shaderHandler{nullptr};

		void begin() const override {

		}

		void end(FrameBuffer* target) const override {
			if(shader == nullptr || toProcess == nullptr || target == nullptr)throwException();
			toProcess->getTexture().active(0);

			if(shaderHandler) {
				Draw::blit(target, shader, shaderHandler);
			}else {
				Draw::blit(target, shader);
			}

		}

		void process() const override {

		}
	};
}
