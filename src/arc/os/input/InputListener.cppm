module;

export module OS.KeyListener;

export namespace OS {
	class KeyListener {
		virtual void inform(int keyCode, int action, const int mods)
	};
}