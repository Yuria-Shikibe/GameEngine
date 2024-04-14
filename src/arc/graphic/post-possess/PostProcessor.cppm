module;

#include <glad/glad.h>

export module Graphic.PostProcessor;

export import GL.Buffer.FrameBuffer;
export import GL.Texture.Texture2D;
export import GL;

import Concepts;

export import ext.RuntimeException;

import std;

using namespace GL;

export namespace Graphic {
	struct FramePort { //TODO uses bitfield to hint what to active
		unsigned int inPort = 0;
		unsigned int outPort = 0;

		friend FramePort& operator |(const FramePort& lhs, FramePort& rhs){
			//begin | p1 | p2 | p3 | p4 | ...
			rhs.inPort = lhs.outPort;
			return rhs;
		}

		// template <typename ...Args>
		// static void link(const Args& args...) requires (std::same_as<Args, FramePort> && ...){
		//
		// }
	};

	struct FrameBufferAdapter {
		FrameBuffer* toProcess{nullptr};

		explicit FrameBufferAdapter(FrameBuffer* toProcess)
			: toProcess(toProcess){
		}

		FrameBuffer* operator()() const{
			return toProcess;
		}
	};

	class PostProcessor {
	protected:
		std::unordered_map<GLenum, bool> targetState{};
		mutable std::unordered_map<GLenum, bool> originalState{};

		virtual void initCurrentState() const {
			for(auto [capFlag, expectedState] : targetState) {
				if(const bool cur = originalState[capFlag] = GL::getState(capFlag); cur != expectedState) {
					GL::setState(capFlag, expectedState);
				}
			}
		}

		virtual void resumeOriginalState() const {
			for(auto [capFlag, expectedState] : originalState) {
				if(const bool cur = GL::getState(capFlag); cur != expectedState) {
					GL::setState(capFlag, expectedState);
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

		virtual void applySrc(FrameBuffer* frame) const {
			if(frame)toProcess = frame;

			beginProcess();
		}

		virtual void beginProcess() const = 0;

		virtual void runProcess() const = 0;

		virtual void endProcess(FrameBuffer* target) const = 0;

		template <
			Concepts::Invokable<FrameBuffer*(FrameBuffer*)> AdapterSrc,
			Concepts::Invokable<FrameBuffer*(FrameBuffer*)> AdapterTgt>
		FrameBuffer* apply(AdapterSrc&& source, AdapterTgt&& target) const {
			initCurrentState();

			this->applySrc(source());
			this->runProcess();
			this->endProcess(target());

			resumeOriginalState();

			return target;
		}

		FrameBuffer* apply(FrameBuffer* source, FrameBuffer* target) const {
			initCurrentState();

			applySrc(source);
			runProcess();
			endProcess(target);

			resumeOriginalState();

			return target;
		}

		[[nodiscard]] FrameBuffer* getToProcess() const {
			return toProcess;
		}


		//TODO pipe process mode support?
		friend FrameBuffer* operator|(FrameBuffer* target, const PostProcessor& processor){
			processor.applySrc(processor.toProcess);
			processor.runProcess();
			processor.endProcess(target);

			return target;
		}

		virtual ~PostProcessor() = default;
	};
}
