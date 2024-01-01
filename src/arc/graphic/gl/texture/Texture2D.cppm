module;

export module GL.Texture.Texture2D;

import Graphic.Resizeable;
import RuntimeException;
import <string>;
import <ostream>;
import <memory>;
import <glad/glad.h>;
import Image;
import File;

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

		std::unique_ptr<unsigned char, STBI_DataDeleter> localData{nullptr};

		Texture2D() = default;

		[[nodiscard]] GLenum getTargetFlag() const {
			return targetFlag;
		}

		~Texture2D() override{ // NOLINT(*-use-equals-default)
			if(textureID > 0)glDeleteTextures(1, &textureID);
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

		void resize(const unsigned int w, const unsigned int h) override;

		[[nodiscard]] bool valid() const {
			return localData != nullptr;
		}

		[[nodiscard]] unsigned getWidth() const {
			return width;
		}

		[[nodiscard]] unsigned getHeight() const {
			return height;
		}

		unsigned char* loadFromFile(const OS::File& file);


		/**
		 * \brief The data will be released after init, uses rv-ref to pass the pointer
		 * \param data
		 */
		void init(unsigned char*&& data);

		void init() {
			init(localData.get());
		}

		void updateData();

		void setParameters(GLint downScale = GL_LINEAR_MIPMAP_LINEAR, GLint upScale = GL_LINEAR, GLint clampX = GL_CLAMP_TO_EDGE, GLint clampY = GL_CLAMP_TO_EDGE) const;

		void setScale(GLint downScale = GL::mipmap_nearest_linear, GLint upScale = GL::linear) const;

		void setWrap(GLint clamp) const;

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

		[[nodiscard]] std::unique_ptr<unsigned char[]> copyData() const;

		[[nodiscard]] GLuint getID() const{
			return textureID;
		}

		void setID(const GLuint id){
			textureID = id;;
		}
	};
}
