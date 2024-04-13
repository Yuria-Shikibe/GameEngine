export module Graphic.Resizeable;

export import Concepts;

export namespace Graphic{
	template <Concepts::Number T>
	struct Resizeable
	{
		virtual ~Resizeable() = default;

		virtual void resize(T w, T h) = 0;
	};

	// using ResizeableUInt = Resizeable<unsigned int>;
	using ResizeableInt = Resizeable<int>;
}


