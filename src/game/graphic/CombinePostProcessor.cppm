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
		mutable GL::FrameBuffer temp{2, 2, 3};

	public:
		std::unique_ptr<Graphic::BloomProcessor> bloomer{nullptr};
		GL::Shader* mergeShader{nullptr};

		CombinePostProcessor(Graphic::BloomProcessor* bloomer, GL::Shader* mergeShader)
			: bloomer(bloomer),
			  mergeShader(mergeShader){

			bloomer->port.inPort = 2;
		}

		void beginProcess() const override{
			temp.resize(toProcess->getWidth(), toProcess->getHeight());

			bloomer->apply(toProcess, &temp);
		}

		void runProcess() const override{

		}

		void endProcess(FrameBuffer* target) const override{
			this->toProcess->bind(GL::FrameBuffer::READ);
			this->toProcess->bindAllColorAttachments();

			Graphic::Draw::blit(target, 0, mergeShader);
		}
	};
}
