//
// Created by Matrix on 2024/4/2.
//

export module OS.TextInputListener;

export namespace OS{
	struct TextInputListener{
		virtual ~TextInputListener() = default;

		virtual void informTextInput(unsigned int codepoint, int mods) = 0;

		virtual void informBackSpace() = 0;
		virtual void informDelete() = 0;

		virtual void snapBackTrace(){}

		virtual void snapForwardTrace(){}
	};
}
