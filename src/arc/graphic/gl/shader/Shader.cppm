module;

#include <glad/glad.h>

export module GL.Shader;

import GL;
import OS.File;

import RuntimeException;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL.Texture;
import GL.Uniform;

import Graphic.Color;
import std;

import Heterogeneous;

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

	//TODO seperate the loader and handle?
	class Shader {
	public:
		struct UniformInfo{
			GLint location{};
			GLsizei count{};
			GLenum type{};
		};

	protected:
		mutable ext::StringMap<UniformInfo> uniformInfoMap{};
		mutable std::unordered_map<ShaderType, std::pair<std::string, std::string>> typeList{};

		bool valid{false};

		GLuint programID = 0;

		OS::File shaderDir{};

		std::function<void(const Shader &)> drawer = [](const Shader&) {};

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

		void bindLoaction() const{
			GLint uniform_count = 0;
			glGetProgramiv(programID, GL_ACTIVE_UNIFORMS, &uniform_count);

			if (uniform_count != 0){
				uniformInfoMap.reserve(uniform_count);
				GLint 	maxUniformLength = 0;
				GLsizei length = 0;
				GLsizei count = 0;
				GLenum 	type = GL_NONE;
				glGetProgramiv(programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLength);


				for (GLint i = 0; i < uniform_count; ++i)
				{
					const auto uniform_name = std::make_unique<char[]>(maxUniformLength);

					glGetActiveUniform(programID, i, maxUniformLength, &length, &count, &type, uniform_name.get());

					auto str = std::string(uniform_name.get(), length);

					uniformInfoMap.try_emplace(
						str,
						glGetUniformLocation(programID, str.c_str()), count
					);

				}
			}
		}

	public:
		[[nodiscard]] GLint getLocation(const std::string_view uniform) const {
#ifdef _DEBUG
			auto itr = uniformInfoMap.find(uniform);
			if(itr != uniformInfoMap.end()){
				return itr->second.location;
			}else return -1;

			//throw ext::IllegalArguments{std::format("Unable To Find Uniform [{}] in Shader: {}", uniform, this->getID())};
#else
			return uniformInfoMap.at(uniform).location;
#endif

		}

		[[nodiscard]] UniformInfo getUniformInfo(const std::string_view uniform) const {
			return uniformInfoMap.at(uniform);
		}

		void reDirectDir(const OS::File& file){
			shaderDir = file;
		}

		using compileTypeList = std::pair<ShaderType, std::string>;

		Shader(const OS::File& directory, std::span<compileTypeList> list) : shaderDir{directory}{
			for (const auto& [type, name] : list) {
				pushSource(type, name);
			}
		}

		Shader(const OS::File& directory, std::initializer_list<compileTypeList> list) : shaderDir{directory}{
			for (const auto& [type, name] : list) {
				pushSource(type, name);
			}
		}

		Shader(const OS::File& directory, const std::string& name, const std::initializer_list<ShaderType> list) : shaderDir{directory}{
			for (const auto& type : list) {
				pushSource(type, name);
			}
		}

		explicit Shader(const OS::File& directory, const std::string& name) : shaderDir{directory}{
			pushSource(ShaderType::vert, name);
			pushSource(ShaderType::frag, name);
		}

		~Shader(){ // NOLINT(*-use-equals-default)
			glDeleteProgram(programID);
		}

		Shader(const Shader& other)
			: uniformInfoMap{ other.uniformInfoMap },
			valid{ other.valid },
			programID{ other.programID },
			drawer{ other.drawer } {
			if(!other.isValid())throw ext::IllegalArguments{"Illegal Operation: Copy Invalid Shader!"};
		}

		Shader& operator=(const Shader& other) {
			if(!other.isValid())throw ext::IllegalArguments{"Illegal Operation: Copy Invalid Shader!"};
			if(this == &other) return *this;
			uniformInfoMap = other.uniformInfoMap;
			valid              = other.valid;
			programID          = other.programID;
			drawer             = other.drawer;
			return *this;
		}

		Shader(Shader&& other) noexcept
			:
			  uniformInfoMap(std::move(other.uniformInfoMap)),
			  typeList(std::move(other.typeList)),
			  valid(other.valid),
			  programID(other.programID),
			  shaderDir(std::move(other.shaderDir)),
			  drawer(std::move(other.drawer)){
		}

		Shader& operator=(Shader&& other) noexcept{
			if(this == &other) return *this;
			uniformInfoMap = std::move(other.uniformInfoMap);
			typeList = std::move(other.typeList);
			valid = other.valid;
			programID = other.programID;
			shaderDir = std::move(other.shaderDir);
			drawer = std::move(other.drawer);
			return *this;
		}

		/**
		 * \brief Compile Methods:
		 * */
		void pushSource(const ShaderType type, const std::string& name) const{
			typeList.insert_or_assign(type, std::make_pair(name + suffix(type), std::string{}));
		}

		void readSource() const {
			for(auto& [file, source] : typeList | std::views::values) {
				std::cout << file << std::endl;

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

			valid = true;
		}

		[[nodiscard]] bool isValid() const {
			return valid;
		}

		void bind() const {
			GL::useProgram(programID);
		}

		// ReSharper disable once CppMemberFunctionMayBeStatic
		void unbind() const {
			GL::useProgram(0);
		}

		void setUniformer(const std::function<void(const Shader&)>& run) {
			drawer = run;
		}

		void setColor(const std::string& name, const Graphic::Color& color) const {
			uniformColor(getLocation(name), color);
		}

		void setBool(const std::string& name, const bool value) const {
			glUniform1i(getLocation(name), static_cast<int>(value));
		}

		// ------------------------------------------------------------------------
		void setInt(const std::string& name, const int value) const {
			glUniform1i(getLocation(name), value);
		}

		// ------------------------------------------------------------------------
		void setFloat(const std::string& name, const float value) const {
			glUniform1f(getLocation(name), value);
		}

		// ------------------------------------------------------------------------
		void setVec2(const std::string& name, const float x, const float y) const {
			glUniform2f(getLocation(name), x, y);
		}

		void setVec2(const std::string& name, const Geom::Vec2 &vector) const {
			setVec2(name, vector.getX(), vector.getY());
		}

		// ------------------------------------------------------------------------
		void setMat3(const std::string& name, const Geom::Matrix3D &mat) const {
			glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, mat.getRawVal());
		}

		void setTexture2D(const std::string& name, const Texture* texture, const int offset = 0) const {
			texture->active(offset);

			uniformTexture(getLocation(name), offset);
		}

		void setTexture2D(const std::string &name, const int offset = 0) const {
			uniformTexture(getLocation(name), offset);
		}

		void apply() const {
			drawer(*this);
		}

		template <Concepts::Invokable<void(const Shader&)> func>
		void applyDynamic(const func& f) const {
			drawer(*this);
			f(*this);
		}

		[[nodiscard]] const std::function<void(const Shader&)>& getDrawer() const {
			return drawer;
		}

		[[nodiscard]] GLuint getID() const {
			return programID;
		}
	};
}


