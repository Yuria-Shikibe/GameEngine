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

import File;

import RuntimeException;

import Geom.Vector2D;
import Geom.Matrix3D;
import GL.GL_Exception;
import GL.Texture.Texture2D;
import GL.Uniform;

import Graphic.Color;

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

	std::string shaderTypeStr(const ShaderType shaderType) {
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
		mutable std::unordered_map<ShaderType, std::string> typeList{};

		GLuint programID = 0;

		const OS::File* shaderDir = nullptr;

		std::function<void(const Shader &)> drawer = [](const Shader&) {};

		template<typename... GLuint>
		static unsigned int attachShaders(GLuint... args) {
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

		static unsigned int attachShadersAll(const std::span<GLuint> &programs) {
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

		static GLuint compile(const std::string &src, const ShaderType shaderType) {
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
					"ERROR::" + shaderTypeStr(shaderType) +
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
		[[nodiscard]] GLint getLocation(const std::string &uniform) const {
			return uniformLocationMap.at(uniform);
		}

		void reDirectDir(const OS::File& file){
			shaderDir = &file;
		}

		using compileTypeList = std::pair<ShaderType, std::string>;

		explicit Shader(const OS::File& directory, std::span<compileTypeList> list) : shaderDir{&directory}{
			std::vector<unsigned int> programs;

			for (const auto& [type, name] : list) {
				programs.push_back(compile(type, name));
			}

			programID = attachShadersAll(programs);
			bindLoaction();
		}

		Shader(const OS::File& directory, std::initializer_list<compileTypeList> list) : shaderDir{&directory}{
			std::vector<unsigned int> programs;

			for (const auto& [type, name] : list) {
				programs.push_back(compile(type, name));
			}

			programID = attachShadersAll(programs);
			bindLoaction();
		}

		Shader(const OS::File& directory, const std::string& name, const std::initializer_list<ShaderType> list) : shaderDir{&directory}{
			std::vector<unsigned int> programs;

			for (const auto& t : list) {
				programs.push_back(compile(t, name));
			}

			programID = attachShadersAll(programs);
			bindLoaction();
		}

		explicit Shader(const OS::File& directory, const std::string& name) : shaderDir{&directory}{
			programID = attachShaders(compile(ShaderType::vert, name), compile(ShaderType::frag, name));
			bindLoaction();
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
		[[nodiscard]] unsigned int compileVertShader(const std::string& name) const{
			typeList.insert_or_assign(ShaderType::vert, name + suffix(ShaderType::vert));
			return compile(shaderDir->find(name + suffix(ShaderType::vert)).readString(multiParser), ShaderType::vert);
		}

		[[nodiscard]] unsigned int compileFragShader(const std::string& name) const{
			typeList.insert_or_assign(ShaderType::frag, name + suffix(ShaderType::frag));
			return compile(shaderDir->find(name + suffix(ShaderType::frag)).readString(uniformParser), ShaderType::frag);
		}

		[[nodiscard]] unsigned int compileGeomShader(const std::string& stemName) const{
			typeList.insert_or_assign(ShaderType::geom, stemName + suffix(ShaderType::geom));
			return compile(shaderDir->find(stemName + suffix(ShaderType::geom)).readString(), ShaderType::geom);
		}

		[[nodiscard]] unsigned int compileTeseShader(const std::string& stemName) const{
			typeList.insert_or_assign(ShaderType::tese, stemName + suffix(ShaderType::tese));
			return compile(shaderDir->find(stemName + suffix(ShaderType::tese)).readString(), ShaderType::tese);
		}

		[[nodiscard]] unsigned int compileTescShader(const std::string& stemName) const{
			typeList.insert_or_assign(ShaderType::tesc, stemName + suffix(ShaderType::tesc));
			return compile(shaderDir->find(stemName + suffix(ShaderType::tesc)).readString(), ShaderType::tesc);
		}

		[[nodiscard]] unsigned int compileCompShader(const std::string& stemName) const{
			typeList.insert_or_assign(ShaderType::comp, stemName + suffix(ShaderType::comp));
			return compile(shaderDir->find(stemName + suffix(ShaderType::comp)).readString(), ShaderType::comp);
		}

		[[nodiscard]] unsigned int compile(const ShaderType type, const std::string& stemName) const{
			switch (type) {
				case ShaderType::vert:
					return compileVertShader(stemName);
				case ShaderType::frag:
					return compileFragShader(stemName);
				case ShaderType::geom:
					return compileGeomShader(stemName);
				case ShaderType::tesc:
					return compileTescShader(stemName);
				case ShaderType::tese:
					return compileTeseShader(stemName);
				case ShaderType::comp:
					return compileCompShader(stemName);
				default:
					return 0;
			}
		}

		void bind() const {
			glUseProgram(programID);
		}

		[[nodiscard]] std::unique_ptr<Shader> copy() const {
			std::vector<std::pair<ShaderType, std::string>> list;

			for (const auto& pair : typeList) {
				list.emplace_back(pair);
			}

			return std::make_unique<Shader>(*shaderDir, list);
		}

		void unbind() const {
			glUseProgram(0);
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

		void setColor(const std::string &name, const Graphic::Color& color) const {
			uniformColor(getLocation(name), color);
		}

		void setBool(const std::string &name, const bool value) const {
			glUniform1i(getLocation(name), static_cast<int>(value));
		}

		// ------------------------------------------------------------------------
		void setInt(const std::string &name, const int value) const {
			glUniform1i(getLocation(name), value);
		}

		// ------------------------------------------------------------------------
		void setFloat(const std::string &name, const float value) const {
			glUniform1f(getLocation(name), value);
		}

		// ------------------------------------------------------------------------
		void setVec2(const std::string &name, const float x, const float y) const {
			glUniform2f(getLocation(name), x, y);
		}

		void setVec2(const std::string &name, const Geom::Vector2D &vector) const {
			setVec2(name, vector.getX(), vector.getY());
		}

		// ------------------------------------------------------------------------
		void setMat3(const std::string &name, const Geom::Matrix3D &mat) const {
			glUniformMatrix3fv(getLocation(name), 1, GL_FALSE, mat.getVal());
		}

		void setTexture2D(const std::string &name, const Texture2D &texture, const int offset = 0) const {
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

		[[nodiscard]] GLuint getID() const {
			return programID;
		}
	};
}


