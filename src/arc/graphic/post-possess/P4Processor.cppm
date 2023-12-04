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

import Assets;

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

		unsigned int processTimes = 3;
		float scale = 1.0f;

	public:
		[[nodiscard]] P4Processor() = default;

		~P4Processor() override = default;

		PostProcessor* ping2pong = nullptr;
		PostProcessor* pong2ping = nullptr;

		[[nodiscard]] FrameBuffer* getPing() const {
			return &ping;
		}

		[[nodiscard]] FrameBuffer* getPong() const {
			return &pong;
		}

		[[nodiscard]] P4Processor(PostProcessor* const ping2PongShader, PostProcessor* const pong2PingShader)
			: ping2pong(ping2PongShader),
			  pong2ping(pong2PingShader) {
		}

		[[nodiscard]] P4Processor(PostProcessor* const ping2Pong, PostProcessor* const pong2Ping, const unsigned processTimes)
			: ping2pong(ping2Pong),
			pong2ping(pong2Ping),
			processTimes(processTimes) {
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

		void begin() const override {
			if(ping2pong == nullptr || ping2pong == nullptr || toProcess == nullptr)throwException();

			ping.resize(toProcess->getWidth() * scale, toProcess->getHeight() * scale);
			pong.resize(toProcess->getWidth() * scale, toProcess->getHeight() * scale);

			ping.clear();
			pong.clear();

			toProcess->getTexture().active();

			Draw::blit(&ping);
		}

		void process() const override {
			for(unsigned int i = 0; i < processTimes; i ++) {
				ping2pong->apply(&ping, &pong);
				pong2ping->apply(&pong, &ping);
			}
		}

		void end(FrameBuffer* target) const override {
			ping.getTexture().active();

			Draw::blit(target);
		}
	};
}
