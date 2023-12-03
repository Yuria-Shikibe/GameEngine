module;

export module Graphic.PostProcessor;

export import GL.Buffer.FrameBuffer;
export import GL.Texture.Texture2D;
export import GL;

import RuntimeException;
import <ranges>;
import <unordered_map>;
import <glad/glad.h>;

using namespace GL;

export namespace Graphic {
	class PostProcessor {
	protected:
		std::unordered_map<GLenum, bool> targetState{};
		mutable std::unordered_map<GLenum, bool> originalState{};

		void initCurrentState() const {
			for(auto [cap, state] : targetState) {
				if(const bool cur = originalState[cap] = GL::getState(cap); cur != state) {
					GL::setState(cap, state);
				}
			}
		}

		void resumeOriginalState() const {
			for(auto [cap, state] : originalState) {
				if(const bool cur = GL::getState(cap); cur != state) {
					GL::setState(cap, state);
				}
			}
		}
	public:
		mutable FrameBuffer* toProcess{nullptr};

		[[nodiscard]] PostProcessor() = default;

		[[nodiscard]] explicit PostProcessor(FrameBuffer* const toProcess)
			: toProcess(toProcess) {
		}

		void setTargetState(const GLenum target, const bool enabled){
			targetState.insert_or_assign(target, enabled);
		}

		void setTargetState(const PostProcessor* const other){
			targetState = other->targetState;
		}

		virtual void throwException() const {
			throw ext::NullPointerException{"Null Pointer Exception For Invalid Arguments!"};
		}

		virtual void begin(FrameBuffer* frame) const {
			if(frame)toProcess = frame;

			begin();
		}

		virtual void begin() const = 0;

		virtual void process() const = 0;

		virtual void end(FrameBuffer* target) const = 0;

		virtual FrameBuffer* apply(FrameBuffer* source, FrameBuffer* target) const {
			initCurrentState();

			begin(source);
			process();
			end(target);

			resumeOriginalState();

			return target;
		}

		[[nodiscard]] FrameBuffer* getToProcess() const {
			return toProcess;
		}


		//TODO pipe process mode support?
		FrameBuffer* operator|(FrameBuffer* target) {
			this->begin(this->toProcess);
			this->process();
			this->end(target);

			return target;
		}

		virtual ~PostProcessor() = default;
	};
}
