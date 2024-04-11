//
// Created by Matrix on 2024/4/9.
//

export module Graphic.Mask;

import GL.Buffer.FrameBuffer;
import GL.Shader;
import GL;

export import Graphic.Color;
export import Graphic.Draw;

export namespace Graphic{
	class Mask{

	protected:
		GL::FrameBuffer maskBuffer{GL::FrameBuffer::RecommendedMinSize, GL::FrameBuffer::RecommendedMinSize};
		GL::FrameBuffer* fallbackBuffer{nullptr};

	public:
		Mask() = default;

		Mask(const Mask& other) = delete;

		Mask(Mask&& other) noexcept = delete;

		Mask& operator=(const Mask& other) = delete;

		Mask& operator=(Mask&& other) noexcept = delete;

		void resize(const unsigned int w, const unsigned int h){
			maskBuffer.resize(w, h);
		}

		void setFallback(GL::FrameBuffer* fallbackBuffer, const bool resizeToIt = true){
			this->fallbackBuffer = fallbackBuffer;
			if(resizeToIt){
				resize(fallbackBuffer->getWidth(), fallbackBuffer->getHeight());
			}
		}

		void clearColor(const Color color = Colors::CLEAR) const {
			maskBuffer.clearColor(color);
		}

		void bind() const{
			maskBuffer.bind();
		}

		void bindFallback() const{
			if(fallbackBuffer)fallbackBuffer->bind();
		}

		[[nodiscard]] GL::FrameBuffer& getMaskBuffer(){ return maskBuffer; }
		[[nodiscard]] const GL::FrameBuffer& getMaskBuffer() const { return maskBuffer; }

		[[nodiscard]] GL::FrameBuffer* getFallbackBuffer() const{ return fallbackBuffer; }
		void setFallbackBuffer(GL::FrameBuffer* const fallbackBuffer){ this->fallbackBuffer = fallbackBuffer; }

		//TODO depth or stentich support maybe
	};
}
