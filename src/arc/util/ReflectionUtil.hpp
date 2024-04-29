//
// Created by Matrix on 2024/4/23.
//

#ifndef REFLECTIONUTIL_HPP
#define REFLECTIONUTIL_HPP

import ext.StaticReflection;
import std;

#define REFL__SRL_ ext::reflect::SrlType::

#define REFL___FLATTEN_FUNC2(x,y) x##y
#define REFL___FLATTEN_FUNC1(x,y) REFL___FLATTEN_FUNC2(x,y)

#define REFL_COUNT __COUNTER__
#define REFL_GEN_NAME(x) REFL___FLATTEN_FUNC1(x,__COUNTER__)

#define REFL_NAMESPACE ext::reflect
#define REFL_NAMESPACE_OF ::REFL_NAMESPACE::


#define REFL_INSTANCE_NAMESPACE REFL_NAMESPACE :: _instantiation
#define REFL_INSTANCE(type) namespace REFL_INSTANCE_NAMESPACE{inline type REFL_GEN_NAME(_instance_){};}


#define REFL_INFO__NAME(Type) static constexpr std::string_view name{#Type};

#define REFL_EXPORT_GLOBAL export

#define REFL_REGISTER_CLASS_BEGIN(Class) \
template<> struct REFL_NAMESPACE_OF ClassInfo<Class>{\
	REFL_INFO__NAME(Class)\

#define REFL_REGISTER_FIELD_BEGIN(Class, Member) \
template<> struct REFL_NAMESPACE_OF FieldInfo<&Class::Member>{\
	REFL_INFO__NAME(Member)\

#define REFL_INFO__SERI(type) static constexpr REFL_NAMESPACE_OF SrlType srlType{type};

#define REFL_TEMPLATE_CONTEXT_MPTR ptr

#define REFL_REGISTER_FIELD_DEF(Class, Member) \
template<> struct REFL_NAMESPACE_OF FieldInfo<&Class::Member>{\
REFL_INFO__NAME(Member)\
REFL_USING_DEF_SERI(Class, Member)\
REFL_REGISTER_END\

#define REFL_REGISTER_FIELD_AS(Class, Member, srl) \
template<> struct REFL_NAMESPACE_OF FieldInfo<&Class::Member>{\
REFL_INFO__NAME(Member)\
REFL_INFO__SERI(REFL__SRL_ srl)\
REFL_REGISTER_END\

#define REFL_USING_DEF_SERI(Class, Member) static constexpr auto srlType{\
ext::reflect::dependencySrlType<ext::GetMemberPtrInfo<decltype(&Class::Member)>::ClassType>\
};\

#define REFL_REGISTER_END };\


#define REFL_REGISTER_RTTI(type) REFL_INSTANCE(REFL_NAMESPACE_OF RTTIRegistor<type>)\



#define  REFL_REGISTER_CLASS_DEF(type) REFL_REGISTER_CLASS_BEGIN(type)\
REFL_INFO__SERI(REFL__SRL_ depends)\
REFL_REGISTER_END\
;\
REFL_REGISTER_RTTI(type)\


#define REFL_REGISTER_FUNDAMENTAL(type) template <>\
struct REFL_NAMESPACE_OF ClassInfo<type>{\
	REFL_INFO__NAME(type)\
	REFL_INFO__SERI(REFL__SRL_ depends);\
};\
template <>\
struct REFL_NAMESPACE_OF ClassInfo<type*>{\
REFL_INFO__NAME(type*)\
REFL_INFO__SERI(REFL__SRL_ disable);\
};\

#define REFL_REGISTER_COMPLETION(class, ...)\
template <>\
struct ext::reflect::ClassField<class> : \
ext::reflect::ClassField<class, ##__VA_ARGS__>{};\

// #define SmartPrintf(fmt, ...) printf(fmt, PP_SmartList(__VA_ARGS__))

REFL_REGISTER_FUNDAMENTAL(char              )
REFL_REGISTER_FUNDAMENTAL(signed char       )
REFL_REGISTER_FUNDAMENTAL(unsigned char     )
REFL_REGISTER_FUNDAMENTAL(wchar_t           )
REFL_REGISTER_FUNDAMENTAL(char8_t           )
REFL_REGISTER_FUNDAMENTAL(char16_t          )
REFL_REGISTER_FUNDAMENTAL(char32_t          )
REFL_REGISTER_FUNDAMENTAL(short             )
REFL_REGISTER_FUNDAMENTAL(unsigned short    )
REFL_REGISTER_FUNDAMENTAL(int               )
REFL_REGISTER_FUNDAMENTAL(unsigned int      )
REFL_REGISTER_FUNDAMENTAL(long              )
REFL_REGISTER_FUNDAMENTAL(unsigned long     )
REFL_REGISTER_FUNDAMENTAL(long long         )
REFL_REGISTER_FUNDAMENTAL(unsigned long long)
REFL_REGISTER_FUNDAMENTAL(float             )
REFL_REGISTER_FUNDAMENTAL(double            )
// REFL_REGISTER_CLASS(::TestClass)
//
//
// REFL_REGISTER_FIELD(::TestClass, val)
// REFL_REGISTER_FIELD_SERI(::TestClass, val, 1, 0)
//
//
// constexpr auto name = ext::reflect::Class<TestClass>::getName;
// constexpr auto fname = ext::reflect::Field<&TestClass::val>::getName;
// constexpr auto srlType = ext::reflect::Field<&TestClass::val>::SerializeInfo::type;

#endif //REFLECTIONUTIL_HPP
