export module OS.ApplicationListener;

export namespace OS{
	struct ApplicationListener{
		bool pauseRestrictable = false;
		virtual ~ApplicationListener() = default;

		virtual void update(float delta){

		}

		virtual void updateGlobal(float delta){

		}

		virtual void updatePost(float delta){

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
