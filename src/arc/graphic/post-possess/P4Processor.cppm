//
// Created by Matrix on 2023/11/26.
//
module;

export module Graphic.PostProcessor.P4Processor;

import <glad/glad.h>;
import Graphic.PostProcessor;
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
	template <Concepts::Derived<TexturePoster> Poster = TexturePoster>
	class P4Processor : virtual public PostProcessor{
	public:
		~P4Processor() override = default;

		FrameBuffer ping{2, 2};
		FrameBuffer pong{2, 2};

		Shader* ping2pongShader = nullptr;
		Shader* pong2pingShader = nullptr;

		Texture2D& pingTexture = ping.getTexture(false);
		Texture2D& pongTexture = pong.getTexture(false);

		unsigned int processTimes = 1;

		[[nodiscard]] P4Processor(FrameBuffer* const toProcess, Shader* const ping2PongShader, Shader* const pong2PingShader)
			: PostProcessor(toProcess),
			  ping2pongShader(ping2PongShader),
			  pong2pingShader(pong2PingShader) {
		}

		void begin() override {
			if(ping2pongShader == nullptr || ping2pongShader == nullptr)throw ext::RuntimeException{"Null Pointer Exception For Invalid Shaders!"};

			ping.resize(toProcess->getWidth(), toProcess->getHeight());
			pong.resize(toProcess->getWidth(), toProcess->getHeight());

			glBindFramebuffer(GL_READ_FRAMEBUFFER, toProcess->getID());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ping.getID());
			glBlitFramebuffer(0, 0, toProcess->getWidth(), toProcess->getHeight(), 0, 0, toProcess->getWidth(), toProcess->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}

		void process() override {
			for(unsigned int i = 0; i < processTimes; i ++) {
				ping2pongShader->bind();
				ping2pongShader->apply();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, ping.getID());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, pong.getID());

				Draw::blit();

				pong2pingShader->bind();
				pong2pingShader->apply();

				glBindFramebuffer(GL_READ_FRAMEBUFFER, pong.getID());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ping.getID());

				Draw::blit();
			}

		}

		void end() override {
			glBindFramebuffer(GL_READ_FRAMEBUFFER, ping.getID());
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, toProcess->getID());
			glBlitFramebuffer(0, 0, toProcess->getWidth(), toProcess->getHeight(), 0, 0, toProcess->getWidth(), toProcess->getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
		}
	};
}