module;

export module OS.InputListener;

export namespace OS {
	class InputListener {
	public:
		virtual ~InputListener() = default;

		virtual void inform(int keyCode, int action, int mods) = 0;
	};
}