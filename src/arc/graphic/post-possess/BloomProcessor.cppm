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
		mutable FrameBuffer temp{2, 2};

		P4Processor blur;
		const Shader* bloomRenderer;
		//Threshold

	public:
		[[nodiscard]] BloomProcessor(PostProcessor* const blurProcessor1, PostProcessor* const blurProcessor2, const GL::Shader* const bloomShader)
			: blur(blurProcessor1, blurProcessor2, 2),
				bloomRenderer(bloomShader) {

			setTargetState(GL_DEPTH_TEST, false);
			setTargetState(GL_BLEND, false);

			blur.setTargetState(this);
		}

		float intensity_blo = 1.0f;
		float intensity_ori = 1.5f;

		bool blending = true;

		void begin() const override {
			temp.resize(toProcess->getWidth(), toProcess->getHeight());
		}

		void process() const override {
			blur.apply(toProcess, &temp);
		}

		void end(FrameBuffer* target) const override {
			target->bind(FrameBuffer::DRAW);

			toProcess->getTexture().active(0);
			temp.getTexture().active(0);

			if(blending){
				GL::enable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			Draw::blit(bloomRenderer, [this](const Shader& shader) {
				shader.setFloat("intensity_blo", intensity_blo);
				shader.setFloat("intensity_ori", intensity_ori);
			});
		}
	};
}