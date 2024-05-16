export module OS.InputListener;

export namespace OS {
	class InputListener {
	public:
		virtual ~InputListener() = default;

		virtual void inform(int key, int action, int mods) = 0;
	};
}