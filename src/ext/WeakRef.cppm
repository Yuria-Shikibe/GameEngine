//
// Created by Matrix on 2024/4/4.
//

export module WeakRef;


export namespace ext{
	template <typename T>
	struct WeakRef{
		T* ptr{nullptr};

		WeakRef(const WeakRef& other)
			: ptr(nullptr){}

		WeakRef(WeakRef&& other) noexcept
			: ptr(other.ptr){}

		WeakRef& operator=(const WeakRef& other){
			if(this == &other) return *this;
			ptr = other.ptr;
			return *this;
		}

		WeakRef& operator=(WeakRef&& other) noexcept{
			if(this == &other) return *this;
			ptr = other.ptr;
			return *this;
		}
	};
}