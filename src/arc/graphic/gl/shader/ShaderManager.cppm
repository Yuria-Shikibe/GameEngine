module;

export module GL.Shader.Manager;

export import GL.Shader;
import Async;
import Assets.Loader;
import std;

export namespace GL { //TODO this isn't a good namespace, I thought
	class ShaderManager final : public ext::ProgressTask<void, Assets::AssetsTaskHandler>{
		std::vector<std::unique_ptr<GL::Shader>> shaders{};

	public:
		GL::Shader* registerShader(GL::Shader* shader) {
			shaders.emplace_back(shader);
			return shader;
		}

		GL::Shader* registerShader(auto&&...args) {
			auto ptr = std::make_unique<GL::Shader>(args...);
			auto retPtr = ptr.get();
			shaders.push_back(std::move(ptr));
			return retPtr;
		}

		void compileAll() {
			std::vector<std::future<void>> futures{};
			futures.reserve(shaders.size());
			float total = static_cast<float>(shaders.size());

			for(auto& shader : shaders) {
				if(shader->isValid()) {
					total -= 1;
					continue;
				}

				shader->readSource();
				futures.emplace_back(postToHandler(std::bind(&GL::Shader::compile, shader.get(), true)));
				taskProgress += 0.5f * 1 / total;
			}

			for(auto& future : futures) {
				future.get();
				taskProgress += 0.5f * 1 / total;
			}

			setDone();
		}

		[[nodiscard]] std::future<void> launch(std::launch policy) override {
			return std::async(policy, std::bind(&ShaderManager::compileAll, this));
		}

		[[nodiscard]] std::string_view getTaskName() const override {
			return "Loading & Compiling Shaders";
		}
	};
}
