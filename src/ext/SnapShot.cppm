//
// Created by Matrix on 2024/6/14.
//

export module ext.SnapShot;

import std;

export namespace ext{
	template <typename T>
		requires (std::is_copy_assignable_v<T>)
	struct SnapShot{
		T cap{};
		T snap{};

		[[nodiscard]] SnapShot() requires (std::is_default_constructible_v<T>) = default;

		[[nodiscard]] explicit SnapShot(const T& ref) : cap{ref}, snap{ref}{}

		void resume() {
			snap = cap;
		}

		template <typename R>
		void resumeProj(R T::* mptr) requires requires{
			requires std::is_copy_assignable_v<std::invoke_result_t<decltype(mptr), T&>>;
		}{
			std::invoke_r<R&>(mptr, snap) = std::invoke_r<const R&>(mptr, cap);
		}

		void apply() {
			cap = snap;
		}

		template <typename R>
		void applyProj(R T::* mptr) requires requires{
			requires std::is_copy_assignable_v<std::invoke_result_t<decltype(mptr), T&>>;
		}{
			std::invoke_r<R&>(mptr, cap) = std::invoke_r<const R&>(mptr, snap);
		}

		void set(const T& val) {
			cap = snap = val;
		}

		SnapShot& operator=(const T& val){
			SnapShot::set(val);
			return *this;
		}

		T* operator ->() noexcept{
			return &cap;
		}

		const T* operator ->() const noexcept{
			return &cap;
		}

		SnapShot(const SnapShot& other) = default;

		SnapShot(SnapShot&& other) noexcept = default;

		SnapShot& operator=(const SnapShot& other) = default;

		SnapShot& operator=(SnapShot&& other) noexcept = default;

		friend bool operator==(const SnapShot& lhs, const SnapShot& rhs) requires requires(const T& t){
			{ t == t } -> std::convertible_to<bool>;
		}{
			return lhs.cap == rhs.cap;
		}
	};
}

