export module OS.ApplicationListener;

export namespace OS{
	class ApplicationListener{
	public:
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

		ApplicationListener(const ApplicationListener& o) = delete;
		ApplicationListener(ApplicationListener&& o) = delete;
		ApplicationListener& operator=(const ApplicationListener& o) = delete;
		ApplicationListener& operator=(ApplicationListener&& o) = delete;
	};
}
