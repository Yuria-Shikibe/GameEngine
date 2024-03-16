module;

export module GL.Buffer.FrameBuffer;

import <memory>;
import <glad/glad.h>;
import GL;
import GL.Texture.Texture2D;
import GL.Object;
import GL.Buffer.RenderBuffer;
import Graphic.Resizeable;
import Graphic.Color;
import RuntimeException;
import <numeric>;
import <ranges>;
import <vector>;

export namespace GL{
	class FrameBuffer : public GLObject, public Graphic::ResizeableUInt
	{
	protected:
		unsigned int width = 0, height = 0;
		std::vector<std::unique_ptr<Texture2D>> attachmentsColor{};
		std::unique_ptr<Texture2D> attachmentsDepth{nullptr};
		std::unique_ptr<Texture2D> attachmentsStencil{nullptr};

		std::unique_ptr<RenderBuffer> renderBuffer{nullptr};

	public:
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

		FrameBuffer(const unsigned int w, const unsigned int h, const int colorAttachments = 1) : GLObject{GL_FRAMEBUFFER}, width(w), height(h){
			glCreateFramebuffers(1, &nameID);


			attachmentsColor.emplace_back(std::make_unique<Texture2D>(width, height));
			glNamedFramebufferTexture(nameID, GL_COLOR_ATTACHMENT0, attachmentsColor.front()->getID(), 0);

			bindRenderBuffer<RenderBuffer>();

			for(const auto& texture : attachmentsColor){
				texture->setParameters(GL::linear, GL::linear);
			}
		}

		template <Concepts::Derived<Texture2D> TexType, typename ...Args>
		void bindColorAttachments(const int colorAttachments = 1, Args... args) {
			attachmentsColor.resize(colorAttachments);

			for(int i = 0; i < getColorAttachmentsCount(); ++i) {
				auto& sample = attachmentsColor.at(i);
				sample = std::make_unique<TexType>(width, height, args...);
				glNamedFramebufferTexture(nameID, GL_COLOR_ATTACHMENT0 + i, attachmentsColor.at(i)->getID(), 0);
			}

			enableMainOnly();
		}

		template <Concepts::Derived<Texture2D> TexType, typename ...Args>
		void bindDepthAttachments(Args... args) {
			attachmentsDepth = std::make_unique<TexType>(width, height, args...);
			glNamedFramebufferTexture(nameID, GL_DEPTH_ATTACHMENT, attachmentsDepth->getID(), 0);
		}

		template <Concepts::Derived<Texture2D> TexType, typename ...Args>
		void bindStencilAttachments(Args... args) {
			attachmentsStencil = std::make_unique<TexType>(width, height, args...);
			glNamedFramebufferTexture(nameID, GL_STENCIL_ATTACHMENT, attachmentsStencil->getID(), 0);
		}

		template <Concepts::Derived<RenderBuffer> RBType, typename ...Args>
		void bindRenderBuffer(Args... args) {
			renderBuffer = std::make_unique<RBType>(width, height, args...);
			glNamedFramebufferRenderbuffer(nameID, GL_DEPTH_STENCIL_ATTACHMENT, renderBuffer->getTargetFlag(), renderBuffer->getID());
		}

		~FrameBuffer() override{
			if(nameID){
				glDeleteFramebuffers(1, &nameID);
				GL::cleari(nameID);
			}
		}

		void enableMainOnly() const {
			glDrawBuffers(1, ALL_COLOR_ATTACHMENTS.data());
		}

		void enableAll() const {
			glDrawBuffers(getColorAttachmentsCount(), ALL_COLOR_ATTACHMENTS.data());
		}

		[[nodiscard]] size_t getColorAttachmentsCount() const {
			return attachmentsColor.size();
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if(w == width && h == height)return;
			width = w;
			height = h;

			for(const auto& sample : attachmentsColor) {
				sample->resize(w, h);
			}

			if(renderBuffer)renderBuffer->resize(w, h);
		}

		[[nodiscard]] bool check() const{
			bind();
			return glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
		}

		void bind() const{
			GL::bindFrameBuffer(targetFlag, nameID);
		}

		void bind(const GLenum mode) const {
			GL::bindFrameBuffer(mode, nameID);
		}

		void unbind() const{
			GL::bindFrameBuffer(targetFlag, 0);
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> readPixelsRaw(const unsigned int width, const unsigned int height, const int srcX = 0, const int srcY = 0) const {
			bind();
			auto pixels = std::make_unique<unsigned char[]>(static_cast<size_t>(width) * height * 4);
			glReadPixels(srcX, srcY, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.get());
			return pixels;
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> readPixelsRaw() const {
			return readPixelsRaw(width, height, 0, 0);
		}

		[[nodiscard]] const std::vector<std::unique_ptr<Texture2D>>& getTextures() const{
			return attachmentsColor;
		}

		[[nodiscard]] std::vector<std::unique_ptr<Texture2D>>& getTextures(){
			return attachmentsColor;
		}

		[[nodiscard]] const Texture2D* operator[](const size_t index) const{
			return attachmentsColor.at(index).get();
		}

		void clear(const Graphic::Color& initColor = Graphic::Colors::CLEAR) const {
			clearColor(initColor);
		}

		void clearColor(const Graphic::Color& initColor = Graphic::Colors::CLEAR, const unsigned attachmentID = 0) const{
			glClearNamedFramebufferfv(nameID, GL_COLOR, attachmentID, initColor.asRaw());
		}

		void clearDepth(const float depth) const{
			if(attachmentsDepth){
				glClearNamedFramebufferfv(nameID, GL_DEPTH, 0, &depth);
			}
		}

		void clearRenderData(const float depth = 1.0f, const int stencil = 0) const{
			if(renderBuffer){
				glClearNamedFramebufferfi(nameID, GL_DEPTH_STENCIL, 0, depth, stencil);
			}
		}

		void clearStencil(const float stencil) const{
			if(attachmentsStencil){
				glClearNamedFramebufferfv(nameID, GL_STENCIL, 0, &stencil);
			}
		}

		Texture2D& getTexture() const {
			return *attachmentsColor.front().get();
		}

		[[nodiscard]] unsigned int getWidth() const { return width; }
		[[nodiscard]] unsigned int getHeight() const { return height; }

	};
}
