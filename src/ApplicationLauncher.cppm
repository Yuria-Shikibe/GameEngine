// //
// // Created by Matrix on 2023/12/5.
// //
//
// module;
//
// export module ApplicationLauncher;
//
// import <memory>;
// import Concepts;
// import OS;
// import OS.ApplicationListener;
// import Core;
//
// export {
// 	enum class ApplicationState {
// 		init,
// 		paused,
// 		running,
// 		disposing
// 	};
//
// 	class ApplicationCore : OS::ApplicationListener{
// 	public:
// 		virtual ~ApplicationCore() = default;
//
// 		virtual void init() {
//
// 		}
//
//
//
// 		template <Concepts::Invokable<void()> Func>
// 		void post(const Func& func) {
// 			OS::post(std::forward<Func>(func));
// 		}
// 	};
//
// }
