//
// Created by Matrix on 2024/6/8.
//

export module Assets.Load.Misc;

import Assets.Load.Core;
import std;
import ext.Concepts;

export namespace Assets::Load{
	template <typename FuncTy = void()>
	// using FuncTy = void();
	class MiscTaskManager : public LoadTask{
		using RetTy = typename Concepts::FunctionTraits<FuncTy>::ReturnType;

		struct MiscTask{
			Phase phase{};
			std::move_only_function<FuncTy> func{};
			std::optional<std::promise<RetTy>> promise{};

			[[nodiscard]] MiscTask(Phase phase, std::move_only_function<FuncTy>&& func,
				std::optional<std::promise<RetTy>>&& promise = std::nullopt)
				: phase{phase},
				  func{std::move(func)},
				  promise{std::move(promise)}{}
		};

		std::vector<MiscTask> tasks{};

		void execCurrentPhase(){
			for (auto& [phase, func, promise] : tasks){
				if(handler.getCurrentPhase() == phase){
					try{
						if constexpr (std::is_void_v<RetTy>){
							func();
							if(promise)promise->set_value();
						}else{
							auto rst = func();
							if(promise)promise->set_value(std::move(rst));
						}
					}catch(...){
						handler.throwException(std::current_exception());
					}

					progress += 1 / static_cast<float>(tasks.size());
				}
			}
		}

		void load(){
			while(!handler.stopToken.stop_requested() && !tasks.empty()){
				execCurrentPhase();
				if(auto* barriar = handler.getBarriar()){
					barriar->arrive_and_wait();
				}
			}

			progress = 1.f;
			handler.join();
		}

	public:
		void push(std::move_only_function<FuncTy>&& function){
			tasks.emplace_back(Phase::clear, std::move(function));
		}

		void push(const Phase requestedPhase, std::move_only_function<FuncTy>&& function){
			tasks.emplace_back(requestedPhase, std::move(function));
		}

		[[nodiscard]] std::future<RetTy> pushAndGet(const Phase requestedPhase, std::move_only_function<FuncTy>&& function){
			auto promise = std::promise<RetTy>{};

			auto future = promise.get_future();

			tasks.emplace_back(requestedPhase, std::move(function), std::move(promise));

			return future;
		}

		[[nodiscard]] std::future<void> launch(std::launch policy) override{
			return std::async(policy, &MiscTaskManager::load, this);
		}

		[[nodiscard]] std::string_view getCurrentTaskName() const noexcept override{
			return "Misc Load Tasks";
		}
	};
}
