//
// Created by Matrix on 2024/4/25.
//

#ifndef REFLECTDATA_BUILTIN_HPP
#define REFLECTDATA_BUILTIN_HPP

#include "../src/arc/util/ReflectionUtil.hpp"

import std;
import ext.StaticReflection;
import Geom.Rect_Orthogonal;
import Math;
import Geom.Vector2D;
import Geom.Transform;
import Graphic.Color;

struct Inner{
	Geom::Transform trans{};
};

struct TestT{
	Geom::Transform transform;

	bool val;
	Graphic::Color color;
	Geom::Vec2 vec2;
	Geom::Point2 point2;
	Geom::OrthoRectFloat quadFloat{};
	Geom::OrthoRectFloat quadFloat2{};
};

#define REG_VEC(type) \
REFL_REGISTER_POD(Geom::Vector2D<type>)\
REFL_REGISTER_FIELD_BEGIN(Geom::Vector2D<type>, x)\
static constexpr ext::reflect::SrlType srlType{ext::reflect::SrlType::depends};\
REFL_REGISTER_END\
;\
REFL_REGISTER_FIELD_BEGIN(Geom::Vector2D<type>, y)\
static constexpr ext::reflect::SrlType srlType{ext::reflect::SrlType::depends};\
REFL_REGISTER_END\



REG_VEC(char              );
REG_VEC(signed char       );
REG_VEC(unsigned char     );
REG_VEC(wchar_t           );
REG_VEC(char8_t           );
REG_VEC(char16_t          );
REG_VEC(char32_t          );
REG_VEC(short             );
REG_VEC(unsigned short    );
REG_VEC(int               );
REG_VEC(unsigned int      );
REG_VEC(long              );
REG_VEC(unsigned long     );
REG_VEC(long long         );
REG_VEC(unsigned long long);
REG_VEC(float             );
REG_VEC(double            );

REFL_REGISTER_POD(Geom::OrthoRectFloat);
REFL_REGISTER_POD(Geom::OrthoRectInt);
REFL_REGISTER_POD(Geom::OrthoRectUInt);

REFL_REGISTER_POD(Geom::Transform);

REFL_REGISTER_POD(Graphic::Color);
REFL_REGISTER_FIELD_DEF(Graphic::Color, r)
REFL_REGISTER_FIELD_DEF(Graphic::Color, g)
REFL_REGISTER_FIELD_DEF(Graphic::Color, b)
REFL_REGISTER_FIELD_DEF(Graphic::Color, a)

REFL_REGISTER_POD(Math::Section<int>);
REFL_REGISTER_POD(Math::Section<unsigned int>);
REFL_REGISTER_POD(Math::Section<float>);

REFL_REGISTER_FIELD_AS(TestT, transform, json)
REFL_REGISTER_FIELD_DEF(TestT, val)
REFL_REGISTER_FIELD_DEF(TestT, vec2)
REFL_REGISTER_FIELD_DEF(TestT, color)
REFL_REGISTER_FIELD_DEF(TestT, point2)
REFL_REGISTER_FIELD_DEF(TestT, quadFloat)
REFL_REGISTER_FIELD_DEF(TestT, quadFloat2)
REFL_REGISTER_COMPLETION(TestT,
	&TestT::transform,
	&TestT::val,
	&TestT::vec2,
	&TestT::color,
	&TestT::point2,
	&TestT::quadFloat,
	&TestT::quadFloat2)

REFL_REGISTER_FIELD_DEF(Geom::Transform, vec)
REFL_REGISTER_FIELD_DEF(Geom::Transform, rot)
REFL_REGISTER_COMPLETION(Geom::Transform, &Geom::Transform::vec, &Geom::Transform::rot)


template <>
struct ext::reflect::ClassField<Inner> :
ext::reflect::ClassField<Inner, &Inner::trans>{};

#endif //REFLECTDATA_BUILTIN_HPP
