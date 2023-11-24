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

		Resizeable() = default;

		Resizeable(const Resizeable& other) = default;

		Resizeable(Resizeable&& other) = default;

		Resizeable& operator=(const Resizeable& other) = default;

		Resizeable& operator=(Resizeable&& other) noexcept = default;
	};

	using ResizeableInt = Resizeable<int>;
}


