//
// Created by Matrix on 2023/11/26.
//
module;

export module Graphic.PostProcessor.P4Processor;

import <glad/glad.h>;
import Graphic.PostProcessor;
import GL;
import GL.Shader;
import RuntimeException;
import Graphic.Draw;
import Concepts;

using namespace GL;

export namespace Graphic {
	/**
	 * \brief Support Customized Blit, if the default Draw cannot meet the requirements;
	 */
	struct TexturePoster {
		virtual ~TexturePoster() = default;

		virtual void operator()(const Texture2D& texture2D){
			Draw::post(texture2D);
		}
	};

	/**
	 * \brief P4 for Ping Pong Post Processor, pairs mode
	 **/
	class P4Processor : public PostProcessor{
	protected:
		mutable FrameBuffer ping{2, 2};
		mutable FrameBuffer pong{2, 2};

	public:
		~P4Processor() override = default;

		PostProcessor* ping2pong = nullptr;
		PostProcessor* pong2ping = nullptr;


		unsigned int processTimes = 1;

		[[nodiscard]] P4Processor(PostProcessor* const ping2PongShader, PostProcessor* const pong2PingShader)
			: ping2pong(ping2PongShader),
			  pong2ping(pong2PingShader) {
		}

		[[nodiscard]] P4Processor(PostProcessor* const ping2Pong, PostProcessor* const pong2Ping, const unsigned processTimes)
			: ping2pong(ping2Pong),
			pong2ping(pong2Ping),
			processTimes(processTimes) {
		}

		void begin() const override {
			if(ping2pong == nullptr || ping2pong == nullptr || toProcess == nullptr)throwException();

			ping.resize(toProcess->getWidth(), toProcess->getHeight());
			pong.resize(toProcess->getWidth(), toProcess->getHeight());

			ping.clear();
			pong.clear();

			Draw::blitRaw(toProcess, &ping);
		}

		void process() const override {
			for(unsigned int i = 0; i < processTimes; i ++) {
				ping2pong->apply(&ping, &pong);
				pong2ping->apply(&pong, &ping);
			}
		}

		void end(FrameBuffer* target) const override {
			Draw::blitRaw(&ping, target);
		}
	};
}