//
// Created by Matrix on 2023/12/3.
//
module;

#include <glad/glad.h>

export module Graphic.PostProcessor.BloomProcessor;

import Graphic.PostProcessor;
import Graphic.PostProcessor.P4Processor;
import Graphic.PostProcessor.ShaderProcessor;
import Graphic.Draw;
import GL.Constants;
import GL.Shader;

using namespace GL;

export namespace Graphic {
	class BloomProcessor : public PostProcessor {
	protected:
		mutable FrameBuffer temp1{2, 2};
		mutable FrameBuffer temp2{2, 2};

		ShaderProcessor thresHolder{};
		P4Processor blur{};
		const Shader* bloomRenderer{nullptr};
		//Threshold

	public:
		[[nodiscard]] BloomProcessor(PostProcessor* const blurProcessor1, PostProcessor* const blurProcessor2, const GL::Shader* const bloomShader, const GL::Shader* const thresHoldShader)
			: thresHolder(thresHoldShader), blur(blurProcessor1, blurProcessor2, 2), bloomRenderer(bloomShader)
		{
			setTargetState(GL_DEPTH_TEST, false);
			setTargetState(GL_BLEND, false);

			blur.setTargetState(this);
			scale = 0.25f;
			blur.setScale(scale);

			thresHolder.shaderHandler = [this](const Shader& shader) {
				shader.setFloat("threshold", threshold);
			};
		}

		float threshold = 0.3f;

		float intensity_blo = 0.9f;
		float intensity_ori = 1.225f;
		float scale = 0.25f;

		bool blending = true;

		void begin() const override {
			temp1.clear();
			temp2.clear();

			temp1.resize(toProcess->getWidth() * scale, toProcess->getHeight() * scale);
			temp2.resize(toProcess->getWidth(), toProcess->getHeight());
			thresHolder.apply(toProcess, &temp2);
		}

		void process() const override {
			blur.apply(&temp2, &temp1);
		}

		void end(FrameBuffer* target) const override {
			toProcess->getTexture().active(0);
			temp1.getTexture().active(1);

			GL::enable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			Draw::blit(target, bloomRenderer, [this](const Shader& shader) {
				shader.setFloat("intensity_blo", intensity_blo);
				shader.setFloat("intensity_ori", intensity_ori);
			});
		}
	};
}