module;

export module Game.Graphic.CombinePostProcessor;

export import Graphic.PostProcessor;
export import Graphic.PostProcessor.ShaderProcessor;
export import Graphic.PostProcessor.BloomProcessor;
export import Graphic.PostProcessor.P4Processor;
export import GL.Shader;
import GL.Buffer.FrameBuffer;

import Graphic.Draw;
import std;

using namespace GL;

export namespace Game{
	class CombinePostProcessor : public ::Graphic::PostProcessor{
	protected:
		mutable GL::FrameBuffer temp{GL::FrameBuffer::RecommendedMinSize, GL::FrameBuffer::RecommendedMinSize};

	public:
		Graphic::PingPongProcessor blur{};

		const GL::Shader* mergeShader{nullptr};
		const GL::Shader* bloomShader{nullptr};
		const GL::Shader* SSAObliter{nullptr};

		CombinePostProcessor(Graphic::PostProcessor* const blurProcessor1, Graphic::PostProcessor* const blurProcessor2, const GL::Shader* mergeShader)
			: blur(blurProcessor1, blurProcessor2, 3),
			  mergeShader(mergeShader){

			blur.port.inPort = 2;
			blur.port.outPort = 0;
			blur.setScale(0.75f);
			// setTargetState(GL::State::BLEND, false);
		}

		void beginProcess() const override{
			temp.resize(toProcess->getWidth(), toProcess->getHeight());
			temp.clearColor();
			// Graphic::Draw::blitCopy(toProcess, 2, &temp, 0);
			blur.apply(toProcess, &temp);
		}

		void runProcess() const override{

		}

		void endProcess(FrameBuffer* target) const override{
			this->toProcess->activeAllColorAttachments();
			temp.getTexture().active(this->toProcess->getColorAttachmentsCount());
			Graphic::Frame::blit(target, 0, mergeShader);
		}
	};
}
