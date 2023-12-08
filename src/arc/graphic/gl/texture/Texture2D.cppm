module;

export module GL.Texture.Texture2D;

import File;
import Graphic.Resizeable;
import RuntimeException;
import <string>;
import <ostream>;
import <memory>;
import <glad/glad.h>;
import Image;

export namespace GL{
	class Texture2D : virtual public Graphic::ResizeableInt
	{

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

		std::string texName{"null"};

		std::unique_ptr<unsigned char, STBI_DataDeleter> localData = nullptr;

		Texture2D() = default;

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}

		~Texture2D() override{ // NOLINT(*-use-equals-default)
			glDeleteTextures(1, &textureID);
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
		Texture2D(w, h, data.release()) {

		}

		Texture2D(const unsigned int w, const unsigned int h) : Texture2D(w, h, nullptr) {

		}

		explicit Texture2D(const OS::File& file){
			loadFromFile(file);
			texName = file.stem();
		}

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

		Texture2D(const Texture2D& other) noexcept: textureID(other.textureID),
											   targetFlag(other.targetFlag),
											   width(other.width),
		                                       height(other.height),
											   bpp(other.bpp),
											   texName(other.texName),
											   localData(other.copyData().release()){
		}

		Texture2D(Texture2D&& other) noexcept: textureID(other.textureID),
		                                       targetFlag(other.targetFlag),
		                                       width(other.width),
											   height(other.height),
		                                       bpp(other.bpp),
		                                       texName(std::move(other.texName)),
		                                       localData(std::move(other.localData)){
		}

		Texture2D& operator=(Texture2D&& other) noexcept{
			if(this == &other) return *this;
			localData = std::move(other.localData);
			texName = std::move(other.texName);
			textureID = other.textureID;
			width = other.width;
			height = other.height;
			bpp = other.bpp;
			return *this;
		}

		void free() {
			localData.reset(nullptr);
		}

		void resize(const unsigned int w, const unsigned int h) override{
			if(w == width && h == height)return;

			free();
			width = w;
			height = h;

			bind();
			glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
			unbind();
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

		void loadFromFile(const OS::File& file){
			//TODO File Support
			init(stbi::loadPng(file, width, height, bpp));
		}


		/**
		 * \brief The data will be released after init, uses rv-ref to pass the pointer
		 * \param data
		 */
		void init(unsigned char*&& data) {
			localData.reset(data);

			if(bpp != 4) {
				throw ext::RuntimeException{"Illegal Bpp Size: " + bpp};
			}

			glGenTextures(1, &textureID);

			active();

			//TODO : Check if needed here.
			glTexImage2D(targetFlag, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());

			glGenerateMipmap(targetFlag);

			setParametersDef();
			free();
		}

		void updateData() { // NOLINT(*-make-member-function-const)
			bind();

			//Avoid unnecessary resize allocate
			if(!valid()) {
				localData.reset(new unsigned char[width * height * 4]);
			}

			glGetTexImage(targetFlag, 0, GL_RGBA, GL_UNSIGNED_BYTE, localData.get());
			unbind();
		}

		void setParametersDef() const{
			bind();
			glTexParameteri(targetFlag, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(targetFlag, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			unbind();
		}

		void setScale(const GLint upScale, const GLint downScale) const{
			bind();
			glTexParameteri(targetFlag, GL_TEXTURE_MIN_FILTER, downScale);
			glTexParameteri(targetFlag, GL_TEXTURE_MAG_FILTER, upScale);
			unbind();
		}

		void setWrap(const GLint clamp) const{
			bind();
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_S, clamp);
			glTexParameteri(targetFlag, GL_TEXTURE_WRAP_T, clamp);
			unbind();
		}

		void bind() const{
			glBindTexture(targetFlag, textureID);
		}
		
		void active(const unsigned char slotOffset = 0) const{ // NOLINT(*-convert-member-functions-to-static)
			glActiveTexture(GL_TEXTURE0 + slotOffset);
			bind();
		}

		void bindParam(const GLenum target) const{
			glBindTexture(target, textureID);
		}

		void unbind() const{
			glBindTexture(targetFlag, 0);
		}

		[[nodiscard]] size_t dataSize() const { //How many bytes!
			if(!localData)return 0;

			return width * height * bpp;
		}

		[[nodiscard]] std::unique_ptr<unsigned char[]> copyData() const {
			if(!valid()) {
				throw ext::RuntimeException{"Texture doesn't contains data!"};
			}

			const size_t size = dataSize();
			auto* data = new unsigned char[size]{0};

			std::copy_n(localData.get(), size, data);

			return std::unique_ptr<unsigned char[]>(data);
		}

		[[nodiscard]] GLuint getID() const{
			return textureID;
		}

		void setID(const GLuint id){
			textureID = id;;
		}
	};
}