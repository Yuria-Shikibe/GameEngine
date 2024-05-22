export module ext.StaticReflection;

export import ext.MetaProgramming;
import ext.Concepts;
import std;
import ext.Owner;

struct NullptrTestT{
	void* val;
};

export namespace ext::reflect{
	inline auto& constructors_RTTI() noexcept{
		static std::unordered_map<std::string_view, std::function<Owner<void*>()>> _constructors;
		return _constructors;
	}

	inline auto& classNames_RTTI() noexcept{
		static std::unordered_map<std::type_index, std::string_view> _classNames_RTTI;
		return _classNames_RTTI;
	}
}


export namespace ext::reflect{
	enum struct SrlType : unsigned char{
		disable,
		binary_all,
		binary_byMember,
		json,
		depends
	};

	template <auto ptr>
	struct FieldInfo{
		static constexpr std::string_view name{"~Unspec"};
		static constexpr SrlType srlType{};
	};

	template <>
	struct FieldInfo<nullptr> : FieldInfo<&NullptrTestT::val>{};

	template <typename T>
	struct ClassInfo{
		static constexpr std::string_view name{};
		static constexpr SrlType srlType{};
	};

	template <typename T>
		requires std::is_default_constructible_v<T> && std::is_nothrow_constructible_v<T>
	struct RTTIRegistor{
		RTTIRegistor(){
			static_assert(requires{
				ClassInfo<T>::name;
				requires ClassInfo<T>::name.data() != nullptr;
			}, "Field Necessary Info Incomplete!");

			constructors_RTTI().insert_or_assign(ClassInfo<T>::name, +[]{return new T;});

			classNames_RTTI().insert_or_assign(typeid(T), ClassInfo<T>::name);
		}
	};

	template <typename T>
	Owner<T*> tryConstruct() noexcept(std::same_as<void, T>){
		if(const std::decay_t<decltype(constructors_RTTI())>::const_iterator itr = constructors_RTTI().find(ClassInfo<T>::name); itr != constructors_RTTI().end() && itr->second){
			return static_cast<T*>(itr->second.operator()());
		}

		return nullptr;
	}

	template <typename T = void>
	Owner<T*> tryConstruct(const std::string_view name) noexcept(std::same_as<void, T>){
		if(const std::decay_t<decltype(constructors_RTTI())>::const_iterator itr = constructors_RTTI().find(name); itr != constructors_RTTI().end() && itr->second){
			return static_cast<T*>(itr->second.operator()());
		}

		return nullptr;
	}

	//TODO
	template <typename T>
	constexpr SrlType dependencySrlType = ext::conditionalVal<ClassInfo<T>::srlType == SrlType::depends, SrlType::json, ClassInfo<T>::srlType>;

	//TODO
	template <SrlType type>
	constexpr bool enableDependency = type != SrlType::disable;

	template <auto ptr>
	struct Field{
	private:
		[[deprecated("Field Missing Srl Info")]] static constexpr void warningSrl(){};
		static constexpr int checkSrl{[]() constexpr {
			// ReSharper disable once CppDeprecatedEntity
			if constexpr (!requires{ FieldInfo<ptr>::srlType; })warningSrl();
			return 0;
		}()};

	public:
		using PtrType = decltype(ptr);
		using Type = typename ext::GetMemberPtrInfo<PtrType>::ValueType;
		using ClassType = typename ext::GetMemberPtrInfo<PtrType>::ClassType;
		using ClassInfo = ClassInfo<Type>;

		static constexpr auto mptr = ptr;

		static constexpr bool accessible = requires(ClassType c){
			c.*mptr;
		};

		static constexpr bool defined = requires{requires FieldInfo<ptr>::name.data() != nullptr;};
		static constexpr std::string_view getName{[]() constexpr {
			if constexpr (defined){
				return FieldInfo<ptr>::name;
			}else{
				return std::string_view{};
			}
		}()};

		static constexpr SrlType getSrlType{[]() constexpr {
			if constexpr (requires{ FieldInfo<ptr>::srlType; }){
				return FieldInfo<ptr>::srlType;
			}else{
				static_assert(requires{FieldInfo<ptr>::srlType;}, "Field Srl Info Incomplete!");
				return SrlType::disable;
			}
		}()};

		static constexpr bool enableSrl = enableDependency<getSrlType>;

		static_assert(getSrlType != SrlType::depends, "Fields Shouldn't have depends attribute at serilization!");
		static_assert(requires{FieldInfo<ptr>::name;}, "Field Necessary Info Incomplete!");
	};

	template <>
	struct Field<nullptr> : Field<&NullptrTestT::val>{};

	template <typename T, auto... mptrs>
		requires (std::same_as<T, typename ext::GetMemberPtrInfo<decltype(mptrs)>::ClassType> && ...)
	struct ClassField{
		using Type = T;

		using Fields = std::tuple<Field<mptrs>...>;

		static constexpr auto memberPtrs = std::make_tuple(mptrs ...);
		static constexpr auto memberCount = sizeof...(mptrs);

		template <std::size_t fieldIndex>
		using FieldAt = std::tuple_element_t<fieldIndex, Fields>;

		static constexpr bool defined = requires{requires ClassInfo<T>::name.data() != nullptr;};
		static constexpr std::string_view getName{[]() constexpr {
			if constexpr (defined){
				return ClassInfo<T>::name;
			}else{
				return std::string_view{};
			}
		}()};

		static constexpr SrlType getSrlType{[]() constexpr {
			if constexpr (requires{ ClassInfo<T>::srlType; }){
				return ClassInfo<T>::srlType;
			}else{
				return SrlType::disable;
			}
		}()};

		template <typename Func>
			requires requires{Func::template with<std::size_t{}>();}
		static constexpr void fieldEach(){
			[] <std::size_t... I> (std::index_sequence<I...>){
				(Func::template with<I>(), ...);
			}(std::make_index_sequence<memberCount>());
		}

		template <typename Func, typename ...Args>
			requires requires(Args... args){
				Func::template with<std::size_t{}>(args ...);
			}
		static constexpr void fieldEach(Args&& ...args){
			[&] <std::size_t... I> (std::index_sequence<I...>){
				(Func::template with<I>(std::forward<Args>(args) ...), ...);
			}(std::make_index_sequence<memberCount>());
		}

	};
}