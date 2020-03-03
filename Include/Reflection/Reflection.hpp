/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#pragma once

/*
    Reflection

    Implementation is based on excellent refl-cpp library:
    - https://github.com/veselink1/refl-cpp
    - https://medium.com/@vesko.karaganev/compile-time-reflection-in-c-17-55c14ee8106b
    - https://veselink1.github.io/blog/cpp/metaprogramming/2019/07/13/refl-cpp-deep-dive.html
*/

// Compile time utilities.
namespace Reflection::Detail
{
    template<typename... Types>
    struct ObjectList
    {
        using TupleType = std::tuple<Types...>;

        constexpr ObjectList(TupleType&& Objects) :
            Objects(Objects)
        {
        }

        template<std::size_t Index>
        constexpr const std::tuple_element_t<Index, TupleType>& Get() const
        {
            return std::get<Index>(Objects);
        }

        const TupleType Objects;
        static constexpr std::size_t Count = sizeof...(Types);
    };

    constexpr ObjectList<> MakeEmptyObjectList()
    {
        return ObjectList<>(std::make_tuple());
    }
}

// Reflection interface.
namespace Reflection
{
    struct TypeInfoBase
    {
        static constexpr const bool Reflected = false;
        static constexpr const auto Name = "<UnknownType>";
        static constexpr const auto Attributes = Reflection::Detail::MakeEmptyObjectList();
        static constexpr const auto Members = Reflection::Detail::MakeEmptyObjectList();
    };

    template<typename Type>
    struct TypeInfo : public TypeInfoBase
    {
    };

    template<typename Type_>
    struct TypeDescription
    {
        using Type = Type_;

        static constexpr const bool Reflected = TypeInfo<Type>::Reflected;
        static constexpr const auto Name = TypeInfo<Type>::Name;
        static constexpr const auto Attributes = TypeInfo<Type>::Attributes;
        static constexpr const auto Members = TypeInfo<Type>::Members;

        template<std::size_t Index>
        constexpr auto Attribute() const -> decltype(Attributes.template Get<Index>())
        {
            return Attributes.template Get<Index>();
        }

        template<std::size_t Index>
        constexpr auto Member() const -> decltype(Members.template Get<Index>())
        {
            return Members.template Get<Index>();
        }
    };

    template<typename Type>
    constexpr TypeDescription<Type> Reflect()
    {
        return {};
    }

    template<typename Type>
    constexpr TypeDescription<Type> Reflect(const Type&)
    {
        return {};
    }

    template<typename Type>
    constexpr bool IsReflected()
    {
        return TypeInfo<Type>::Reflected;
    }

    template<typename Type>
    constexpr bool IsReflected(const Type& type)
    {
        return TypeInfo<Type>::Reflected;
    }
}

// Attribute handling.
namespace Reflection
{
    struct TypeAttribute
    {
    };

    struct FieldAttribute
    {
    };

    struct MethodAttribute
    {
    };
}

namespace Reflection::Detail
{
    template<typename FirstAttribute, typename... Attributes>
    constexpr bool ValidateAttributeUniqueness()
    {
        constexpr bool result = (... && (!std::is_same_v<FirstAttribute, Attributes> && ValidateAttributeUniqueness<Attributes...>()));
        static_assert(result, "Detected duplicated attribute types!");
        return result;
    }

    template<typename Requirement, typename... Attributes>
    constexpr bool ValidateAttributeUsage()
    {
        constexpr bool result = (... && std::is_base_of_v<Requirement, Attributes>);
        static_assert(result, "Detected incorrect attribute usage!");
        return result;
    }

    template<typename Requirement>
    constexpr ObjectList<> MakeAttributeList()
    {
        return MakeEmptyObjectList();
    }

    template<typename Requirement, typename... Attributes>
    constexpr ObjectList<Attributes...> MakeAttributeList(Attributes&&... attributes)
    {
        ValidateAttributeUniqueness<Attributes...>();
        ValidateAttributeUsage<Requirement, Attributes...>();

        return ObjectList<Attributes...>(std::make_tuple(attributes...));
    }
}

// Member handling.
namespace Reflection::Detail
{
    template<typename Type, std::size_t MemberIndex>
    using MemberEntry = typename TypeInfo<Type>::template MemberInfo<MemberIndex>;

    template<typename Type, std::size_t... MemberIndices>
    constexpr ObjectList<MemberEntry<Type, MemberIndices>...> MakeMemberList(std::index_sequence<MemberIndices...>)
    {
        return ObjectList<MemberEntry<Type, MemberIndices>...>(std::make_tuple(MemberEntry<Type, MemberIndices>()...));
    }
}

// Macro helpers.
#define REFLECTION_EXPAND(x) x
#define REFLECTION_STRINGIFY(expression) #expression

// Type declaration macros.
#define REFLECTION_TYPE_INFO_BEGIN(Type_) \
    template<> struct Reflection::TypeInfo<Type_> : public TypeInfoBase \
    { \
    private: \
        static constexpr std::size_t MemberIndexOffset = __COUNTER__ + 1; \
    public: \
        using Type = Type_; \
        static constexpr const bool Reflected = true; \
        static constexpr const std::string_view Name = REFLECTION_STRINGIFY(Type_); \
        template<std::size_t, typename Dummy = void> struct MemberInfo;

#define REFLECTION_ATTRIBUTES(...) \
        static constexpr const auto Attributes = Reflection::Detail::MakeAttributeList<Reflection::TypeAttribute>(__VA_ARGS__);

#define REFLECTION_TYPE_END \
        static constexpr const std::size_t MemberCount = __COUNTER__ - MemberIndexOffset; \
        static constexpr const auto Members = Reflection::Detail::MakeMemberList<Type>(std::make_index_sequence<MemberCount>()); \
    };

#define REFLECTION_TYPE_BASE_BEGIN(Type) \
    REFLECTION_TYPE_INFO_BEGIN(Type)

#define REFLECTION_TYPE_DERIVED_BEGIN(DerivedType, BaseType) \
    REFLECTION_TYPE_INFO_BEGIN(DerivedType)

#define REFLECTION_TYPE_BEGIN_DEDUCE(arg1, arg2, arg3, ...) arg3
#define REFLECTION_TYPE_BEGIN_CHOOSER(...) REFLECTION_EXPAND(REFLECTION_TYPE_BEGIN_DEDUCE(__VA_ARGS__, REFLECTION_TYPE_DERIVED_BEGIN, REFLECTION_TYPE_BASE_BEGIN))
#define REFLECTION_TYPE_BEGIN(...) REFLECTION_EXPAND(REFLECTION_TYPE_BEGIN_CHOOSER(__VA_ARGS__)(__VA_ARGS__))

// Field declaration macros.
#define REFLECTION_FIELD_BEGIN(Field) \
    template<typename Dummy> struct MemberInfo<__COUNTER__ - MemberIndexOffset, Dummy> \
    { \
        static constexpr std::string_view Name = REFLECTION_STRINGIFY(Field); \
        static constexpr auto Pointer = &Type::Field; \
        using Type = decltype(Type::Field);

#define REFLECTION_FIELD_ATTRIBUTES(...) \
        static constexpr auto Attributes = Reflection::Detail::MakeAttributeList<Reflection::FieldAttribute>(__VA_ARGS__);

#define REFLECTION_FIELD_END \
    };

#define REFLECTION_FIELD(Field, ...) \
    REFLECTION_FIELD_BEGIN(Field) \
    REFLECTION_FIELD_ATTRIBUTES(__VA_ARGS__) \
    REFLECTION_FIELD_END