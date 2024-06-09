module;

#include <glad/glad.h>

export module GL.Texture.Texture2D;

import Graphic.Resizeable;
import ext.RuntimeException;
import Image;
import OS.File;
import GL.Object;
export import GL.Texture;

import std;

export namespace GL{

	class Texture2D : public GL::Texture
	{
	public:
		static constexpr auto MipMapGeneralLevel = 4;
		static constexpr auto StandardBPP = 4;

	protected:
		auto loadFromFile(const OS::File& file){
			//TODO File Support
			int bpp{};
			return ext::loadPng(file, width, height, bpp);
		}

		Texture2D* next{nullptr};

	public:
		std::unique_ptr<unsigned char[]> localData{nullptr};

		Texture2D() : Texture(GL_TEXTURE_2D){}

		~Texture2D() override{ // NOLINT(*-use-equals-default)
			if(nameID)glDeleteTextures(1, &nameID);
		}

		/**
		 * \brief WARNING! the data will be release after load, copy the data if needed!
		 * \param w
		 * \param h
		 * \param data
		 */
		Texture2D(const int w, const int h, const unsigned char* data, const bool initInstantly = true) : Texture(GL_TEXTURE_2D, w, h){
			if(initInstantly)init(data);
		}

		Texture2D(const int w, const int h, std::unique_ptr<unsigned char[]>&& data, const bool initInstantly = true) : Texture(GL_TEXTURE_2D, w, h), localData{std::move(data)}{
			if(initInstantly){
				init();
			}
		}

		Texture2D(const int w, const int h) : Texture2D(w, h, nullptr) {

		}

		explicit Texture2D(const OS::File& file, const bool initInstantly = true) : Texture(GL_TEXTURE_2D) {
			localData = loadFromFile(file);
			if(initInstantly)init();
		}

		friend bool operator==(const Texture2D& lhs, const Texture2D& rhs){
			return lhs.nameID == rhs.nameID;
		}

		friend bool operator!=(const Texture2D& lhs, const Texture2D& rhs){
			return !(lhs == rhs);
		}

		Texture2D(const Texture2D& other) = delete;

		Texture2D(Texture2D&& other) noexcept
			: GL::Texture(std::move(other)),
			  next(other.next),
			  localData(std::move(other.localData)){
		}

		Texture2D& operator=(const Texture2D& other) = delete;

		Texture2D& operator=(Texture2D&& other) noexcept{
			if(this == &other) return *this;
			GL::Texture::operator =(std::move(other));
			next = other.next;
			localData = std::move(other.localData);
			return *this;
		}

		void free() {
			localData.reset(nullptr);
		}

		void freeGpuData() {
			if(nameID){
				glDeleteTextures(1, &nameID);
			}
		}

		void resize(const int w, const int h) override{
			if(w == width && h == height)return;

			free();
			width  = w;
			height = h;

			throw ext::RuntimeException{"According to new GL req, textures shouldn't change!"};

			// glTexImage2D(targetFlag, 0, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
			// glTextureStorage2D(nameID, 1, GL_RGBA8, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
		}

		[[nodiscard]] bool valid() const {
			return localData != nullptr;
		}

		/**
		 * \brief The data will be released after init, uses rv-ref to pass the pointer
		 * \param data
		 */
		void init(const unsigned char* data){
			glCreateTextures(targetFlag, 1, &nameID);
			glTextureStorage2D(nameID, MipMapGeneralLevel, GL_RGBA8, width, height);

			if(data)glTextureSubImage2D(nameID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
			//TODO : Check if needed here.
			setWrap();
			glGenerateTextureMipmap(nameID);
		}

		void loadData(unsigned char data[]) const{
			if(data){
				glTextureSubImage2D(nameID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
		}

		void init() {
			init(localData.get());
		}

		void active(const unsigned slotOffset) const override{ // NOLINT(*-convert-member-functions-to-static)

			glBindTextureUnit(slotOffset, nameID);
		}

		void activeAll(const unsigned slotOffset) const override{
			glBindTextureUnit(slotOffset, nameID);

			if(next){
				next->activeAll(slotOffset + 1);
			}
		}

		Texture2D* linkTo(Texture2D* next){
			this->next = next;
			return next;
		}

		[[nodiscard]] int dataSize() const { //How many bytes!
			return width * height * StandardBPP;
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> copyData() const{
			if(!valid()) {
				return loadGPUData();
			}

			const size_t size = dataSize();
			auto ptr = std::make_unique<unsigned char[]>(size);

			std::memcpy(ptr.get(), localData.get(), size);

			return ptr;
		}


		[[nodiscard]] std::unique_ptr<unsigned char[]> loadGPUData() const { // NOLINT(*-make-member-function-const)
			const int size = dataSize();

			auto ptr = std::make_unique<unsigned char[]>(size);

			glGetTextureImage(nameID, 0, GL_RGBA, GL_UNSIGNED_BYTE, static_cast<GLsizei>(size), ptr.get());

			return ptr;
		}
	};
}






