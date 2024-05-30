export module OS.ApplicationListener;

export import Core.Unit;

export namespace OS{
	struct ApplicationListener{
		bool pauseRestrictable = false;
		virtual ~ApplicationListener() = default;

		virtual void update(Core::Tick delta){

		}

		virtual void updateGlobal(Core::Tick delta){

		}

		virtual void updatePost(Core::Tick delta){

		}

		ApplicationListener() = default;

		friend bool operator==(const ApplicationListener& lhs, const ApplicationListener& rhs){
			return &lhs == &rhs;
		}

		friend bool operator!=(const ApplicationListener& lhs, const ApplicationListener& rhs){
			return &lhs != &rhs;
		}

		ApplicationListener(const ApplicationListener&) = delete;
		ApplicationListener(ApplicationListener&&) = delete;
		ApplicationListener& operator=(const ApplicationListener&) = delete;
		ApplicationListener& operator=(ApplicationListener&&) = delete;
	};
}
