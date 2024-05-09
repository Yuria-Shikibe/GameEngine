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
			if(!shader)ShaderProcessor::throwException();
		}

		[[nodiscard]] ShaderProcessor() : ShaderProcessor(Graphic::Frame::blitter) { // NOLINT(*-use-equals-default)

		}

		[[nodiscard]] ShaderProcessor(const GL::Shader* const shader,
		                              const std::function<void(const GL::Shader&)>& shaderHandler)
			: shader(shader),
			shaderHandler(shaderHandler) {
			if(!shader)ShaderProcessor::throwException();
		}

		const GL::Shader* shader{nullptr};
		std::function<void(const GL::Shader&)> shaderHandler{nullptr};

		FramePort port{};

		void beginProcess() const override {}

		void endProcess(FrameBuffer* target) const override {
			if(shader == nullptr || toProcess == nullptr || target == nullptr)throwException();
			toProcess->getColorAttachments().at(port.inPort)->active(0);
			toProcess->bind(GL::FrameBuffer::READ);
			// toProcess->bindAllColorAttachments();

			if(shaderHandler) {
				Graphic::Frame::blit(target, port.outPort, shader, shaderHandler);
			}else {
				Graphic::Frame::blit(target, port.outPort, shader, nullptr);
			}
		}

		void runProcess() const override {}
	};
}
