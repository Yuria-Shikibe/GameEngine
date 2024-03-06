module ;

export module Graphic.Resizeable;

export import Concepts;

export namespace Graphic{
	template <Concepts::Number T>
	class Resizeable
	{
	public:
		virtual ~Resizeable() = default;

		[[maybe_unused]] virtual void resize(T w, T h) = 0;
	};

	using ResizeableUInt = Resizeable<unsigned int>;
	using ResizeableInt = Resizeable<int>;
}


