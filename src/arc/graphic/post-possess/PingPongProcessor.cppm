module;

export module Graphic.PostProcessor.P4Processor;

import Graphic.PostProcessor;
import GL;
import GL.Shader;
import RuntimeException;
import Graphic.Draw;
import Concepts;

using namespace GL;

export namespace Graphic {
	/**
	 * \brief P4 for Ping Pong Post Processor, pairs mode
	 **/
	class PingPongProcessor : public PostProcessor{
	protected:
		mutable FrameBuffer ping{2, 2};
		mutable FrameBuffer pong{2, 2};

		unsigned int processTimes = 3;

		float scale = 1.0f;

	public:
		FramePort port{};
		[[nodiscard]] PingPongProcessor() = default;

		~PingPongProcessor() override = default;

		PostProcessor* ping2pong = nullptr;
		PostProcessor* pong2ping = nullptr;

		[[nodiscard]] FrameBuffer* getPing() const {
			return &ping;
		}

		[[nodiscard]] FrameBuffer* getPong() const {
			return &pong;
		}

		[[nodiscard]] PingPongProcessor(PostProcessor* const ping2PongShader, PostProcessor* const pong2PingShader)
			: ping2pong(ping2PongShader),
			  pong2ping(pong2PingShader) {
		}

		[[nodiscard]] PingPongProcessor(PostProcessor* const ping2Pong, PostProcessor* const pong2Ping, const unsigned processTimes)
			: processTimes(processTimes),
			ping2pong(ping2Pong),
			pong2ping(pong2Ping) {
		}

		[[nodiscard]] unsigned getProcessTimes() const {
			return processTimes;
		}

		void setProcessTimes(const unsigned processTimes) {
			this->processTimes = processTimes;
		}

		[[nodiscard]] float getScale() const {
			return scale;
		}

		void setScale(const float scale) {
			if(scale <= 0)throw ext::IllegalArguments{"Negative Scale!"};
			this->scale = scale;
		}

		void beginProcess() const override {
			if(ping2pong == nullptr || ping2pong == nullptr || toProcess == nullptr)throwException();

			ping.resize(toProcess->getWidth() * scale, toProcess->getHeight() * scale);
			pong.resize(toProcess->getWidth() * scale, toProcess->getHeight() * scale);

			ping.clear();
			pong.clear();

			toProcess->getTextures().at(port.inPort)->active(0);

			Graphic::Frame::blit(&ping);
		}

		void runProcess() const override {
			for(unsigned int i = 0; i < processTimes; i ++) {
				ping2pong->apply(&ping, &pong);
				pong2ping->apply(&pong, &ping);
			}
		}

		void endProcess(FrameBuffer* target) const override {
			ping.getTexture().active(0);

			Graphic::Frame::blit(target, port.outPort);
		}
	};
}
