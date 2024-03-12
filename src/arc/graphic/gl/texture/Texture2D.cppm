module;

export module GL.Texture.Texture2D;

import Graphic.Resizeable;
import RuntimeException;
import Image;
import OS.File;

import <string>;
import <ostream>;
import <memory>;
import <glad/glad.h>;


export namespace GL{
	enum TexParams{
		nearest                = GL_NEAREST,
		linear                 = GL_LINEAR,
		mipmap_linear_linear   = GL_LINEAR_MIPMAP_LINEAR,
		mipmap_nearest_nearest = GL_NEAREST_MIPMAP_NEAREST,
		mipmap_nearest_linear  = GL_NEAREST_MIPMAP_LINEAR,
		mipmap_linear_nearest  = GL_LINEAR_MIPMAP_NEAREST,
		filterMag              = GL_TEXTURE_MAG_FILTER,
		filterMin              = GL_TEXTURE_MIN_FILTER,
		wrap_U                 = GL_TEXTURE_WRAP_S,
		wrap_V                 = GL_TEXTURE_WRAP_T,
		//...
	};

	class Texture2D : public Graphic::ResizeableUInt
	{
	public:
		static constexpr GLuint MipMapGeneralLevel = 4;

	protected:
		GLuint textureID = 0;
		GLenum targetFlag = GL_TEXTURE_2D;
		unsigned int width = 0, height = 0;
		unsigned int bpp{4};

	public:
		struct STBI_DataDeleter {
			void operator()(unsigned char* t) const { // NOLINT(*-non-const-parameter)
				if(t) {
					stbi::free(t);
				}
			}
		};

		//TODO is this really necessay?
		std::string texName{"null"};

		std::unique_ptr<unsigned char[], STBI_DataDeleter> localData{nullptr};

		Texture2D() = default;

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}

		~Texture2D() override{ // NOLINT(*-use-equals-default)
			if(textureID)glDeleteTextures(1, &textureID);
		}

		/**
		 * \brief WARNING! the data will be release after load, copy the data if needed!
		 * \param w
		 * \param h
		 * \param data
		 */
		Texture2D(const unsigned int w, const unsigned int h, unsigned char*&& data): width(w), height(h){
			init(std::forward<unsigned char*>(data));
		}

		Texture2D(const unsigned int w, const unsigned int h, std::unique_ptr<unsigned char[]>&& data) :
		Texture2D(w, h, data.release()) {}

		Texture2D(const unsigned int w, const unsigned int h) : Texture2D(w, h, nullptr) {

		}

		explicit Texture2D(const OS::File& file, bool initInstantly = true);

		static std::size_t hash_value(const Texture2D& obj){
			return obj.textureID;
		}

		friend std::ostream& operator<<(std::ostream& os, const Texture2D& obj){
			return os << "Texture: " << obj.texName;
		}

		friend bool operator==(const Texture2D& lhs, const Texture2D& rhs){
			return lhs.texName == rhs.texName;
		}

		friend bool operator!=(const Texture2D& lhs, const Texture2D& rhs){
			return !(lhs == rhs);
		}

		Texture2D(const Texture2D& other) noexcept;

		Texture2D(Texture2D&& other) noexcept;

		Texture2D& operator=(Texture2D&& other) noexcept;

		Texture2D& operator=(const Texture2D& other) = delete;

		void free() {
			localData.reset(nullptr);
		}

		void resize(unsigned int w, unsigned int h) override{
			if(w == width && h == height)return;

			free();
			width  = w;
			height = h;

			glTextureStorage2D(textureID, 1, GL_RGBA8, width, height);
		}

		[[nodiscard]] bool valid() const {
			return localData != nullptr;
		}

		[[nodiscard]] unsigned getWidth() const {
			return width;
		}

		[[nodiscard]] unsigned getHeight() const {
			return height;
		}

		unsigned char* loadFromFile(const OS::File& file){
			//TODO File Support
			return stbi::loadPng(file, width, height, bpp);
		}


		/**
		 * \brief The data will be released after init, uses rv-ref to pass the pointer
		 * \param data
		 */
		void init(unsigned char*&& data){
			if(data != localData.get())localData.reset(data);

			if(bpp != 4) {
				throw ext::RuntimeException{"Illegal Bpp Size: " + bpp};
			}

			glCreateTextures(targetFlag, 1, &textureID);
			glTextureStorage2D(textureID, MipMapGeneralLevel, GL_RGBA8, width, height);

			if(localData)glTextureSubImage2D(textureID, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
			//TODO : Check if needed here.
			setParameters();
			glGenerateTextureMipmap(textureID);
		}

		void init() {
			init(localData.get());
		}

		void setParameters(GLint downScale = GL::mipmap_linear_linear, GLint upScale = GL::linear, GLint clampX = GL_CLAMP_TO_EDGE, GLint clampY = GL_CLAMP_TO_EDGE) const{
			glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, downScale);
			glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, upScale);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, clampX);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, clampY);
		}

		void setScale(const GLint downScale = GL::mipmap_nearest_linear, const GLint upScale = GL::linear) const{
			glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, downScale);
			glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, upScale);
		}

		void setWrap(const GLint clamp) const{
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, clamp);
			glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, clamp);
		}

		void bind() const{
			glBindTexture(targetFlag, textureID);
		}

		void active(const unsigned char slotOffset = 0) const{ // NOLINT(*-convert-member-functions-to-static)
			glBindTextureUnit(slotOffset, textureID);
		}

		void bindParam(const GLenum target) const{
			glBindTexture(target, textureID);
		}

		void unbind() const{
			glBindTexture(targetFlag, 0);
		}

		[[nodiscard]] unsigned dataSize() const { //How many bytes!
			return width * height * bpp;
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
			const unsigned int size = dataSize();

			auto ptr = std::make_unique<unsigned char[]>(size);

			glGetTextureImage(textureID, 0, GL_RGBA, GL_UNSIGNED_BYTE, size, ptr.get());

			return ptr;
		}

		[[nodiscard]] GLuint getID() const{
			return textureID;
		}

		void setID(const GLuint id){ // NOLINT(*-convert-member-functions-to-static)
			textureID = id;;
		}
	};
}
