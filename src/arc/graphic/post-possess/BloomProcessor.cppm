export module Graphic.PostProcessor.BloomProcessor;

import Graphic.PostProcessor;
import Graphic.PostProcessor.P4Processor;
import Graphic.PostProcessor.ShaderProcessor;
import Graphic.Draw;
import GL.Constants;
import GL.Shader;

using namespace GL;

export namespace Graphic {
	/**
	 * @brief Threshold -> Blur -> BloomBlend
	 */
	class BloomProcessor : public PostProcessor {
	protected:
		mutable FrameBuffer temp1{2, 2};

	public:
		FramePort port{};

		PingPongProcessor blur{};

		const Shader* bloomShader{nullptr};
		const Shader* thresHoldShader{nullptr};

		[[nodiscard]] BloomProcessor(PostProcessor* const blurProcessor1, PostProcessor* const blurProcessor2, const GL::Shader* const bloomShader, const GL::Shader* const thresHoldShader)
			: blur(blurProcessor1, blurProcessor2, 3), bloomShader(bloomShader), thresHoldShader(thresHoldShader)
		{
			setTargetState(GL_DEPTH_TEST, false);
			setTargetState(GL_BLEND, false);

			blur.setTargetState(this);
			blur.setScale(1.0f);
		}

		float threshold = 0.3f;

		float intensity_blo = 0.9f;
		float intensity_ori = 1.225f;
		float scale = 0.5f;

		void setIntensity(const float intensity) {
			intensity_ori = intensity;
			intensity_blo = intensity * 0.732f;
		}

		void setIntensity(const float ori, const float blo) {
			intensity_ori = ori;
			intensity_blo = blo;
		}

		void setScale(const float scale) {
			this->scale = scale;
			blur.setScale(scale);
		}

		bool blending = true;

		void beginProcess() const override {
			temp1.resize(toProcess->getWidth() * scale, toProcess->getHeight() * scale);
			temp1.clearColor();

			toProcess->getTextures().at(port.inPort)->active(0);
			Graphic::Frame::blit(&temp1, 0, thresHoldShader, [this](const Shader& shader) {
				shader.setFloat("threshold", threshold);
			});
		}

		void runProcess() const override {
			blur.apply(&temp1, &temp1);
		}

		void endProcess(FrameBuffer* target) const override {
			toProcess->getTextures().at(port.inPort)->active(0);

			temp1.getTexture().active(1);

			GL::enable(GL_BLEND);
			GL::blendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			Graphic::Frame::blit(target, port.outPort, bloomShader, [this](const Shader& shader) {
				shader.setFloat("intensity_blo", intensity_blo);
				shader.setFloat("intensity_ori", intensity_ori);
			});
		}
	};
}