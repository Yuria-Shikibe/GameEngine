module;

#include <glad/glad.h>

export module GL.Buffer.FrameBuffer;

import GL;
import GL.Texture.Texture2D;
import GL.Object;
import GL.DepthBuffer;
import GL.Buffer.RenderBuffer;
import Graphic.Resizeable;
import Graphic.Color;
import ext.RuntimeException;
import std;

export namespace GL{
	class FrameBuffer : public GLObject, public Graphic::ResizeableInt
	{
	protected:
		int width = 0, height = 0;
		std::vector<std::unique_ptr<Texture2D>> attachmentsColor{};
		std::unique_ptr<DepthBuffer> attachmentsDepth{nullptr};
		std::unique_ptr<Texture2D> attachmentsStencil{nullptr};

		std::unique_ptr<RenderBuffer> renderBuffer{nullptr};

	public:
		static constexpr int RecommendedMinSize = 100;
		static constexpr std::array<GLenum, 32> ALL_COLOR_ATTACHMENTS {
			[] {
				std::array<GLenum, 32> arr{};

				for(int i = 0; i < 32; ++i) {
					arr[i] = GL_COLOR_ATTACHMENT0 + i;
				}

				return arr;
			}()
		};

		static constexpr GLenum DEF = GL_FRAMEBUFFER;
		static constexpr GLenum DRAW = GL_DRAW_FRAMEBUFFER;
		static constexpr GLenum READ = GL_READ_FRAMEBUFFER;

		FrameBuffer() : GLObject{GL_FRAMEBUFFER}{}

		FrameBuffer(const int w, const int h, const int colorAttachments = 1) : GLObject{GL_FRAMEBUFFER}, width(w), height(h){
			glCreateFramebuffers(1, &nameID);

			setColorAttachments<Texture2D>(colorAttachments);
			setRenderBuffer<RenderBuffer>();

			for(const auto& texture : attachmentsColor){
				texture->setParameters(GL::linear, GL::linear);
			}
		}

		FrameBuffer(const int w, const int h, Concepts::Invokable<void(FrameBuffer&)> auto&& init) : GLObject{GL_FRAMEBUFFER}, width(w), height(h){
			glCreateFramebuffers(1, &nameID);

			init(*this);

			for(const auto& texture : attachmentsColor){
				texture->setParameters(GL::linear, GL::linear);
			}

			if(!check()){
				throw ext::IllegalArguments{"Invalid Frame Buffer!"};
			}
		}

		template <Concepts::Derived<Texture2D> TexType, typename ...Args>
		void setColorAttachments(const int colorAttachments = 1, Args... args) {
			attachmentsColor.clear();
			attachmentsColor.resize(colorAttachments);

			for(int i = 0; i < getColorAttachmentsCount(); ++i) {
				auto& sample = attachmentsColor.at(i);
				sample = std::make_unique<TexType>(width, height, args...);
				glNamedFramebufferTexture(nameID, GL_COLOR_ATTACHMENT0 + i, attachmentsColor.at(i)->getID(), 0);
			}
		}

		template <Concepts::Derived<DepthBuffer> TexType, typename ...Args>
		void setDepthAttachments(Args... args) {
			attachmentsDepth = std::make_unique<TexType>(width, height, args...);
			glNamedFramebufferTexture(nameID, GL_DEPTH_ATTACHMENT, attachmentsDepth->getID(), 0);
		}

		template <Concepts::Derived<Texture2D> TexType, typename ...Args>
		void setStencilAttachments(Args... args) {
			attachmentsStencil = std::make_unique<TexType>(width, height, args...);
			glNamedFramebufferTexture(nameID, GL_STENCIL_ATTACHMENT, attachmentsStencil->getID(), 0);
		}

		template <Concepts::Derived<RenderBuffer> RBType, typename ...Args>
		void setRenderBuffer(Args... args) {
			renderBuffer = std::make_unique<RBType>(width, height, args...);
			glNamedFramebufferRenderbuffer(nameID, GL_DEPTH_STENCIL_ATTACHMENT, renderBuffer->getTargetFlag(), renderBuffer->getID());
		}

		~FrameBuffer() override{
			if(nameID){
				glDeleteFramebuffers(1, &nameID);
				// GL::cleari(nameID);
			}
		}

		void enableDrawMainOnly() const {
			glDrawBuffers(1, ALL_COLOR_ATTACHMENTS.data());
		}

		void enableDrawAll() const {
			glDrawBuffers(getColorAttachmentsCount(), ALL_COLOR_ATTACHMENTS.data());
		}

		void enableDraw(const int count = 1) const {
			glDrawBuffers(count, ALL_COLOR_ATTACHMENTS.data());
		}

		void enableDrawAt(const int id = 1) const {
			glDrawBuffer(id + GL_COLOR_ATTACHMENT0);
		}

		void enableRead(const int id = 0) const {
			glReadBuffer(id + GL_COLOR_ATTACHMENT0);
		}

		static void enableDrawAll(const std::initializer_list<GLenum> list){
			std::vector<GLenum> indices(list.size());

			std::ranges::transform(list, indices.begin(), [](const int i){
				return i + GL_COLOR_ATTACHMENT0;
			});

			glDrawBuffers(static_cast<GLsizei>(indices.size()), indices.data());
		}

		[[nodiscard]] int getColorAttachmentsCount() const {
			return static_cast<int>(attachmentsColor.size());
		}

		void resize(const int w, const int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;

			setColorAttachments<Texture2D>(getColorAttachmentsCount());

			for(const auto& texture : attachmentsColor){
				texture->setParameters(GL::linear, GL::linear);
			}

			if(renderBuffer)renderBuffer->resize(w, h);
		}

		[[nodiscard]] bool check() const{
			return glCheckNamedFramebufferStatus(nameID, targetFlag) == GL_FRAMEBUFFER_COMPLETE;
		}

		void bind() const{
			GL::bindFrameBuffer(targetFlag, nameID);
			GL::viewport(getWidth(), getHeight());
		}

		void bind(const GLenum mode) const {
			GL::bindFrameBuffer(mode, nameID);
			if(targetFlag == DRAW || targetFlag == DEF){
				GL::viewport(getWidth(), getHeight());
			}
		}

		void unbind() const{
			GL::bindFrameBuffer(targetFlag, 0);
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> readPixelsRaw(const int width, const int height, const int srcX = 0, const int srcY = 0) const {
			bind();
			auto pixels = std::make_unique<unsigned char[]>(static_cast<size_t>(width) * height * 4);
			glReadPixels(srcX, srcY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
			return pixels;
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> readPixelsRaw() const {
			return readPixelsRaw(width, height, 0, 0);
		}

		[[nodiscard]] const std::vector<std::unique_ptr<Texture2D>>& getColorAttachments() const{
			return attachmentsColor;
		}

		[[nodiscard]] std::vector<std::unique_ptr<Texture2D>>& getColorAttachments(){
			return attachmentsColor;
		}

		[[nodiscard]] const Texture2D* operator[](const size_t index) const{
			return attachmentsColor.at(index).get();
		}

		void clear(const Graphic::Color& initColor = Graphic::Colors::CLEAR) const {
			clearColor(initColor);
		}

		void clearColor(const Graphic::Color& initColor = Graphic::Colors::CLEAR, const unsigned attachmentID = 0) const{
			glClearNamedFramebufferfv(nameID, GL_COLOR, static_cast<GLint>(attachmentID), initColor.asRaw());
		}

		void clearColorAll(const Graphic::Color& initColor = Graphic::Colors::CLEAR) const{
			for(int i = 0; i < attachmentsColor.size(); ++i){
				glClearNamedFramebufferfv(nameID, GL_COLOR, i, initColor.asRaw());
			}
		}

		void clearDepth(const float depth = 1.0f) const{
			glClearNamedFramebufferfv(nameID, GL_DEPTH, 0, &depth);
		}

		void clearRenderData(const float depth = 1.0f, const int stencil = 0) const{
			glClearNamedFramebufferfi(nameID, GL_DEPTH_STENCIL, 0, depth, stencil);
		}

		void clearStencil(const float stencil = 0) const{
			if(attachmentsStencil){
				glClearNamedFramebufferfv(nameID, GL_STENCIL, 0, &stencil);
			}
		}

		[[nodiscard]] Texture2D& getTexture() const {
			return *attachmentsColor.front().get();
		}

		void activeAllColorAttachments() const{
			for(int i = 0; i < attachmentsColor.size(); ++i){
				attachmentsColor.at(i)->active(i);
			}
		}

		[[nodiscard]] int getWidth() const { return width; }
		[[nodiscard]] int getHeight() const { return height; }

	};
}
