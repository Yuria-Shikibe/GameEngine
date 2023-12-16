//
// Created by Matrix on 2023/11/18.
//
module;

export module GL.Shader;

import <glad/glad.h>;
import <filesystem>;
import <iostream>;
import <functional>;
import <type_traits>;
import <string>;
import <span>;
import <set>;
import <utility>;

import GL;
import File;

import RuntimeException;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL.GL_Exception;
import GL.Texture.Texture2D;
import GL.Uniform;

import Graphic.Color;
import <ranges>;

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

	std::string_view shaderTypeStr(const ShaderType shaderType) {
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

	std::string suffix(const ShaderType shaderType) {
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

	inline GLuint typeID(ShaderType type){
		return static_cast<GLuint>(type);
	}

	class Shader {
	protected:
		mutable std::unordered_map <std::string, GLint> uniformLocationMap{};
		mutable std::unordered_map <std::string, GLint> attribuLocationMap{};
		mutable std::unordered_map<ShaderType, std::pair<std::string, std::string>> typeList{};

		bool valid{false};

		GLuint programID = 0;

		const OS::File* shaderDir = nullptr;

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

		static GLuint compileCode(const std::string &src, const ShaderType shaderType) {
			const GLuint shader = glCreateShader(typeID(shaderType));
			const char *vert = src.c_str();

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
					"ERROR::" + static_cast<std::string>(shaderTypeStr(shaderType)) +
					"::COMPILATION_FAILED\n" +
                    std::string(infoLog)
				);
			}

			return shader;
		}

		void bindLoaction() const {
			for(auto& [key, pos] : uniformLocationMap) {
				if(pos == -1) {
					pos = glGetUniformLocation(programID, key.data());
				}
			}
		}

	public:
		[[nodiscard]] GLint getLocation(const std::string_view &uniform) const {
			return uniformLocationMap.at(static_cast<std::string>(uniform));
		}

		void reDirectDir(const OS::File& file){
			shaderDir = &file;
		}

		using compileTypeList = std::pair<ShaderType, std::string>;

		explicit Shader(const OS::File& directory, std::span<compileTypeList> list) : shaderDir{&directory}{
			for (const auto& [type, name] : list) {
				pushSource(type, name);
			}
		}

		Shader(const OS::File& directory, std::initializer_list<compileTypeList> list) : shaderDir{&directory}{
			std::vector<unsigned int> programs;

			for (const auto& [type, name] : list) {
				pushSource(type, name);
			}
		}

		Shader(const OS::File& directory, const std::string& name, const std::initializer_list<ShaderType> list) : shaderDir{&directory}{
			for (const auto& type : list) {
				pushSource(type, name);
			}
		}

		explicit Shader(const OS::File& directory, const std::string& name) : shaderDir{&directory}{
			pushSource(ShaderType::vert, name);
			pushSource(ShaderType::frag, name);
		}

		~Shader(){ // NOLINT(*-use-equals-default)
			glDeleteProgram(programID);
		}

		Shader &operator=(const Shader &other) = delete;

		Shader &operator=(Shader &&other) = delete;

		Shader(const Shader &tgt) = delete;

		Shader(Shader &&tgt) = delete;

		struct ShaderSourceParser {
			const std::string keyWord{};
			std::unordered_map<std::string, GLint>& target;

			[[nodiscard]] ShaderSourceParser(const std::string& keyWord, std::unordered_map<std::string, GLint>& target)
				: keyWord(keyWord), target(target) {}

			void operator()(const std::string& line) const {
				if (
					line.find("//") == std::string::npos && line.find(keyWord) != std::string::npos
				){
					const size_t endLine = line.find_last_of(';');
					const size_t beginWord = line.find_last_of(' ') + 1;

					target[line.substr(beginWord, endLine - beginWord)] = -1;

				}
			}
		}uniformParser{"uniform", uniformLocationMap}, attribuParser{"attribute", attribuLocationMap};

		struct ShaderSourceParserMulti {
			std::vector<ShaderSourceParser> parsers{};

			[[nodiscard]] explicit ShaderSourceParserMulti(const std::initializer_list<ShaderSourceParser> list){
				for(auto parser: list)parsers.push_back(parser);
			}

			void operator()(const std::string& line) const {
				for(const auto& parser: parsers)parser(line);
			}
		};

		ShaderSourceParserMulti multiParser = ShaderSourceParserMulti{uniformParser, attribuParser};
		/**
		 * \brief Compile Methods:
		 * */
		void pushSource(const ShaderType type, const std::string& name) const{
			typeList.insert_or_assign(type, std::make_pair(name + suffix(type), std::string{}));
		}

		void readSource() const {
			for(auto& [file, source] : typeList | std::views::values) {
				source = shaderDir->find(file).readString(uniformParser);
			}
		}

		void compile(const bool freeSource = true) {
			std::vector<GLuint> code{};
			code.reserve(typeList.size());

			for(const auto& [type, data] : typeList) {
				code.push_back(compileCode(data.second, type));
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

		//TODO Automatize this function during initialization
		void registerUniform(const std::string &param) const{
			uniformLocationMap.insert_or_assign(param, glGetUniformLocation(programID, param.data()));
		}

		template<typename ...T>
		void registerUniformMulti(const T&... args) const{
			(registerUniform(args), ...);
		}

		void setUniformer(const std::function<void(const Shader&)>& run) {
			drawer = run;
		}

		void setColor(const std::string_view &name, const Graphic::Color& color) const {
			uniformColor(getLocation(name), color);
		}

		void setBool(const std::string_view &name, const bool value) const {
			glUniform1i(getLocation(name), static_cast<int>(value));
		}

		// ------------------------------------------------------------------------
		void setInt(const std::string_view &name, const int value) const {
			glUniform1i(getLocation(name), value);
		}

		// ------------------------------------------------------------------------
		void setFloat(const std::string_view &name, const float value) const {
			glUniform1f(getLocation(name), value);
		}

		// ------------------------------------------------------------------------
		void setVec2(const std::string_view &name, const float x, const float y) const {
			glUniform2f(getLocation(name), x, y);
		}

		void setVec2(const std::string_view &name, const Geom::Vector2D &vector) const {
			setVec2(name, vector.getX(), vector.getY());
		}

		// ------------------------------------------------------------------------
		void setMat3(const std::string_view &name, const Geom::Matrix3D &mat) const {
			glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, mat.getRawVal());
		}

		void setTexture2D(const std::string_view &name, const Texture2D &texture, const int offset = 0) const {
			texture.active(offset);

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


