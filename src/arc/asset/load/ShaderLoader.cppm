//
// Created by Matrix on 2024/6/10.
//

export module Assets.Load.ShaderLoader;

import Assets.Load.Core;
import std;
import GL.Shader;
import ext.Owner;

export namespace Assets::Load{
	//TODO separate source and handle
	class ShaderLoader final : public LoadTask{
		std::vector<std::unique_ptr<GL::ShaderSource>> loadedShaders{};

	public:
		GL::ShaderSource* registerShader(ext::Owner<GL::ShaderSource*> shader) {
			return loadedShaders.emplace_back(shader).get();
		}

		template <typename ...Args>
		GL::ShaderSource* emplaceShader(Args&&... args) {
			auto ptr = std::make_unique<GL::ShaderSource>(std::forward<Args>(args)...);
			auto retPtr = ptr.get();
			loadedShaders.push_back(std::move(ptr));
			return retPtr;
		}

		bool compileAll() {
			std::vector<std::future<void>> futures{};
			futures.reserve(loadedShaders.size());

			for(const auto& [index, shader] : loadedShaders | std::views::enumerate) {
				if(shader->isValid()) {
					continue;
				}

				try{
					shader->readSource();
				}catch(...){
					handler.throwException(std::current_exception());
				}

				futures.emplace_back(handler.postTask([shader = shader.get()]{
					shader->compile(true);
				}));

				progress = 0.5f * index / static_cast<float>(loadedShaders.size());
			}

			for(const auto& [index, future] : futures | std::views::enumerate) {
				try{
					future.get();
				}catch(...){
					handler.throwException(std::current_exception());
				}

				progress = 0.5f * (index / static_cast<float>(futures.size()) + 1);
			}

			done();
			handler.join();

			return finished;
		}

		[[nodiscard]] std::future<bool> launch(const std::launch policy) override {
			return std::async(policy, std::bind(&ShaderLoader::compileAll, this));
		}

		[[nodiscard]] std::string_view getCurrentTaskName() const noexcept override {
			return "Loading & Compiling Shaders";
		}
	};
}
