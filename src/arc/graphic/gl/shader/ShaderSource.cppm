module;

#include <glad/glad.h>

export module GL.Shader;

import GL;
import OS.File;

import ext.RuntimeException;

import Geom.Vector2D;
import Geom.Vector3D;
import Geom.Matrix3D;
import GL.Texture;
import GL.Uniform;

import Graphic.Color;
import std;

import ext.Heterogeneous;

export namespace GL {
	enum class ShaderType : GLuint {
		glsl = GL_SHADER,
		vert = GL_VERTEX_SHADER,
		frag = GL_FRAGMENT_SHADER,
		geom = GL_GEOMETRY_SHADER,
		tese = GL_TESS_EVALUATION_SHADER,
		tesc = GL_TESS_CONTROL_SHADER,
		comp = GL_COMPUTE_SHADER
	};

	constexpr std::string_view shaderTypeStr(const ShaderType shaderType) {
		switch (shaderType) {
			case ShaderType::glsl:
				return "GENERAL";
			case ShaderType::vert:
				return "VERTEX";
			case ShaderType::frag:
				return "FRAGMENT";
			case ShaderType::geom:
				return "GEOMETRY";
			case ShaderType::tesc:
				return "TESS_CONTROL";
			case ShaderType::tese:
				return "TESS_EVALUATION";
			case ShaderType::comp:
				return "COMPUTE";
			default:
				return "UNDEFINED SHADER";
		}
	}

	constexpr std::string suffix(const ShaderType shaderType) {
		switch (shaderType) {
			case ShaderType::glsl:
				return ".glsl";
			case ShaderType::vert:
				return ".vert";
			case ShaderType::frag:
				return ".frag";
			case ShaderType::geom:
				return ".geom";
			case ShaderType::tesc:
				return ".tesc";
			case ShaderType::tese:
				return ".tese";
			case ShaderType::comp:
				return ".comp";
			default:
				return ".txt";
		}
	}

	constexpr GLuint typeID(ShaderType type){
		return static_cast<GLuint>(type);
	}

	class ShaderProgram{
	public:
		struct UniformInfo{
			GLint location{};
			GLsizei count{};
			GLenum type{};
		};

	protected:
		ext::StringHashMap<UniformInfo> uniformInfoMap{};
		GLuint programID = 0;
		std::function<void(const ShaderProgram&)> uniformSetter = [](const ShaderProgram&){};

	public:
		[[nodiscard]] ext::StringHashMap<UniformInfo>& getUniformInfoMap() noexcept{ return uniformInfoMap; }

		[[nodiscard]] GLuint getProgramId() const noexcept{ return programID; }

		[[nodiscard]] auto& getUniformer() const noexcept{ return uniformSetter; }
		[[nodiscard]] auto& getUniformer() noexcept{ return uniformSetter; }

		void setUniformer(const std::function<void(const ShaderProgram&)>& uniformSetter) noexcept{
			this->uniformSetter = uniformSetter;
		}

		[[nodiscard]] GLint getLocation(const std::string_view uniform) const {
			if(const auto itr = uniformInfoMap.find(uniform); itr != uniformInfoMap.end()){
				return itr->second.location;
			}

#if DEBUG_CHECK
			//TODO post a warning
#endif

			return -1;
		}

		[[nodiscard]] UniformInfo getUniformInfo(const std::string_view uniform) const {
			return uniformInfoMap.at(uniform);
		}

		[[nodiscard]] bool isValid() const {
			return programID != 0;
		}

		void bind() const {
			GL::useProgram(programID);
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		void unbind() const {
			GL::useProgram(0);
		}

		void setColor(const std::string_view name, const Graphic::Color& color) const {
			if(const auto location = getLocation(name); location >= 0){
				uniformColor(location, color);
			}
		}

		void setBool(const std::string_view name, const bool value) const {
			if(const auto location = getLocation(name); location >= 0){
				glUniform1i(location, static_cast<int>(value));
			}
		}

		// ------------------------------------------------------------------------
		void setInt(const std::string_view name, const int value) const {
			if(const auto location = getLocation(name); location >= 0){
				glUniform1i(location, value);
			}
		}

		// ------------------------------------------------------------------------
		void setUint(const std::string_view name, const unsigned value) const {
			if(const auto location = getLocation(name); location >= 0){
				glUniform1ui(location, value);
			}
		}

		// ------------------------------------------------------------------------
		void setFloat(const std::string_view name, const float value) const {
			if(const auto location = getLocation(name); location >= 0){
				glUniform1f(location, value);
			}
		}

		// ------------------------------------------------------------------------
		void setFloat3Arr(const std::string_view name, const float value[], const std::size_t count) const {
			auto* info = uniformInfoMap.tryFind(name);
			if(!info)return;
			if(count > info->count){
				throw ext::ArrayIndexOutOfBound{std::format("Cannot Accept Size Larger Than GPU Allocated Size :{} > {}", count, info->count)};
			}
			glUniform3fv(info->location, count, value);
		}

		// ------------------------------------------------------------------------
		void setVec2Arr(const std::string_view name, const Geom::Vec2 value[], const std::size_t count) const {
			auto* info = uniformInfoMap.tryFind(name);
			if(!info)return;
			if(count > info->count){
				throw ext::ArrayIndexOutOfBound{std::format("Cannot Accept Size Larger Than GPU Allocated Size :{} > {}", count, info->count)};
			}
			glUniform2fv(info->location, count, reinterpret_cast<const GLfloat*>(value));
		}

		// ------------------------------------------------------------------------
		void setVec3Arr(const std::string_view name, const Geom::Vector3D<float> value[], const std::size_t count) const {
			auto* info = uniformInfoMap.tryFind(name);
			if(!info)return;
			if(count > info->count){
				throw ext::ArrayIndexOutOfBound{std::format("Cannot Accept Size Larger Than GPU Allocated Size :{} > {}", count, info->count)};
			}
			glUniform3fv(info->location, count, reinterpret_cast<const GLfloat*>(value));
		}

		// ------------------------------------------------------------------------
		void setVec2(const std::string_view name, const float x, const float y) const {
			if(const auto location = getLocation(name); location >= 0){
				glUniform2f(location, x, y);
			}
		}

		void setVec2(const std::string_view name, const Geom::Vec2 vector) const {
			setVec2(name, vector.x, vector.y);
		}

		// ------------------------------------------------------------------------
		void setMat3(const std::string_view name, const Geom::Matrix3D &mat) const {
			if(const auto location = getLocation(name); location >= 0){
				glUniformMatrix3fv(location, 1, GL_FALSE, mat.getRawVal());
			}
		}

		void setTexture2D(const std::string_view name, const Texture* texture, const int offset = 0) const {
			if(const auto location = getLocation(name); location >= 0){
				if(texture)texture->active(offset);
				uniformTexture(location, offset);
			}
		}

		void setTexture2D(const std::string_view name, const int offset = 0) const {
			setTexture2D(name, nullptr, offset);
		}

		void apply() const {
			uniformSetter(*this);
		}

		void applyDynamic(Concepts::Invokable<void(const ShaderProgram&)> auto&& f, const bool invokeSelfDrawer = false) const {
			if(invokeSelfDrawer)uniformSetter(*this);
			f(*this);
		}

		[[nodiscard]] GLuint getID() const {
			return programID;
		}
	};

	//TODO seperate the loader and handle?
	class ShaderSource : public ShaderProgram{
		template<typename... GLuint>
		[[nodiscard]] static unsigned int attachShaders(GLuint... args) {
			int success;
			// link shaders
			const unsigned int shaderProgram = glCreateProgram();

			(glAttachShader(shaderProgram, args), ...);

			glLinkProgram(shaderProgram);
			// check for linking errors
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success) {
				constexpr unsigned int exceptionLength = 1024;
				char infoLog[exceptionLength];
				glGetProgramInfoLog(shaderProgram, exceptionLength, nullptr, infoLog);
				throw ext::RuntimeException("ERROR::PROGRAM::LINKING_FAILED\n" + std::string(infoLog));
			}

			return shaderProgram;
		}

		[[nodiscard]] static unsigned int attachShadersAll(const std::span<GLuint> &programs) {
			int success;
			// link shaders
			const unsigned int shaderProgram = glCreateProgram();

			for (const auto id: programs) {
				glAttachShader(shaderProgram, id);
			}

			glLinkProgram(shaderProgram);
			// check for linking errors
			glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
			if (!success) {
				constexpr unsigned int exceptionLength = 1024;
				char infoLog[exceptionLength];
				glGetProgramInfoLog(shaderProgram, exceptionLength, nullptr, infoLog);
				throw ext::RuntimeException("ERROR::PROGRAM::LINKING_FAILED\n" + std::string(infoLog));
			}

			return shaderProgram;
		}

		[[nodiscard]] static GLuint compileCode(const std::string_view name, const std::string_view src, const ShaderType shaderType) {
			const GLuint shader = glCreateShader(typeID(shaderType));
			const char *vert = src.data();

			glShaderSource(shader, 1, &vert, nullptr);
			glCompileShader(shader);
			// check for shader compile errors
			int success;

			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
			if (!success) {
				constexpr unsigned int exceptionLength = 1024;
				char infoLog[exceptionLength];
				glGetShaderInfoLog(shader, exceptionLength, nullptr, infoLog);
				throw ext::RuntimeException(
					std::format("{} : {}\n\n{}", name, shaderTypeStr(shaderType), infoLog)
				);
			}

			return shader;
		}

		//TODO should compiled shader contains these info??
		OS::File shaderDir{};
		std::unordered_map<ShaderType, std::pair<std::string, std::string>> typeList{};

		void bindLoaction(){
			GLint uniform_count = 0;
			glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &uniform_count);

			if (uniform_count != 0){
				uniformInfoMap.reserve(uniform_count);
				GLint 	maxUniformLength = 0;
				GLsizei nameSize = 0;
				GLsizei count = 0;
				GLenum 	type = GL_NONE;

				glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength);

				const auto uniform_name = std::make_unique<char[]>(maxUniformLength);

				for (GLint i = 0; i < uniform_count; ++i){
					glGetActiveUniform(programID, i, maxUniformLength, &nameSize, &count, &type, uniform_name.get());

					auto str = std::string(uniform_name.get(), nameSize);
					auto location = glGetUniformLocation(programID, str.c_str());

					uniformInfoMap.try_emplace(
						std::move(str),
						location, count
					);
				}
			}
		}

	public:
		void reDirectDir(const OS::File& file){
			shaderDir = file;
		}

		using compileTypeList = std::pair<ShaderType, std::string>;

		[[nodiscard]] ShaderSource() = default;

		ShaderSource(const OS::File& directory, std::span<compileTypeList> list) : shaderDir{directory}{
			for (const auto& [type, name] : list) {
				pushSource(type, name);
			}
		}

		ShaderSource(const OS::File& directory, std::initializer_list<compileTypeList> list) : shaderDir{directory}{
			for (const auto& [type, name] : list) {
				pushSource(type, name);
			}
		}

		ShaderSource(const OS::File& directory, const std::string_view name, const std::initializer_list<ShaderType> list) : shaderDir{directory}{
			for (const auto& type : list) {
				pushSource(type, std::string(name));
			}
		}

		ShaderSource(const OS::File& directory, const std::string_view name) : shaderDir{directory}{
			pushSource(ShaderType::vert, std::string(name));
			pushSource(ShaderType::frag, std::string(name));
		}

		~ShaderSource(){ // NOLINT(*-use-equals-default)
			if(programID != 0)glDeleteProgram(programID);
		}

		/**
		 * \brief Compile Methods:
		 * */
		void pushSource(const ShaderType type, const std::string& name){
			typeList.insert_or_assign(type, std::make_pair(name + suffix(type), std::string{}));
		}

		void readSource() {
			for(auto& [file, source] : typeList | std::views::values) {

				source = shaderDir.find(file).readString();
			}
		}

		void compile(const bool freeSource = true){
			std::vector<GLuint> code{};
			code.reserve(typeList.size());

			for(const auto& [type, data] : typeList) {
				code.push_back(compileCode(data.first, data.second, type));
			}

			programID = attachShadersAll(code);
			bindLoaction();

			if(freeSource)typeList.clear();
		}
	};
}


