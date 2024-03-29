/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include "Reflection/ReflectionDetail.hpp"

/*
    Reflection Static

    Compile-time reflection interface.
*/

namespace Reflection
{
    template<typename ReflectedType, typename AttributeType, std::size_t AttributeIndex>
    struct AttributeDescription
    {
        using Type = std::decay_t<AttributeType>;
        static constexpr auto TypeInfo = Detail::TypeInfo<AttributeType>{};
        static constexpr auto Index = AttributeIndex;
        static constexpr auto Name = TypeInfo.Name;

        template<typename OtherType>
        static constexpr bool IsType()
        {
            return std::is_same<Type, OtherType>::value;
        }
    };

    template<typename ReflectedType, typename AttributeType, std::size_t AttributeIndex>
    struct AttributeDescriptionWithInstance :
        public AttributeDescription<ReflectedType, AttributeType, AttributeIndex>
    {
        using TypeWithoutInstance = AttributeDescription<
            ReflectedType, AttributeType, AttributeIndex>;

        constexpr AttributeDescriptionWithInstance(AttributeType&& instance) :
            Instance(instance)
        {
        }

        const AttributeType Instance;
    };

    template<typename ReflectedType, typename MemberType, std::size_t MemberIndex>
    struct MemberDescription
    {
        using Type = MemberType;
        static constexpr auto TypeInfo =
            Detail::TypeInfo<ReflectedType>::Members.template Get<MemberIndex>();
        static constexpr MemberKind Kind = TypeInfo.Kind;
        static constexpr auto Index = MemberIndex;
        static constexpr auto Name = TypeInfo.Name;
        static constexpr auto Pointer = TypeInfo.Pointer;
        static constexpr auto Attributes =
            Detail::MakeAttributeDescriptionWithInstanceList<ReflectedType>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());
        static constexpr auto AttributeTypes =
            Detail::MakeAttributeDescriptionWithoutInstanceList<ReflectedType>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());

        template<typename OtherType>
        static constexpr bool IsType()
        {
            return std::is_same<Type, OtherType>::value;
        }

        static constexpr MemberKind GetKind()
        {
            return Kind;
        }

        static constexpr bool IsField()
        {
            return Kind == Reflection::Field;
        }

        static constexpr bool IsMethod()
        {
            return Kind == Reflection::Method;
        }

        static constexpr bool HasAttributes()
        {
            return Attributes.Count > 0;
        }

        template<typename AttributeType>
        static constexpr bool HasAttribute()
        {
            return TypeInfo.Attributes.template Contains<AttributeType>();
        }

        template<std::size_t AttributeIndex>
        static constexpr auto Attribute() -> decltype(Attributes.template Get<AttributeIndex>())
        {
            return Attributes.template Get<AttributeIndex>();
        }

        template<auto& AttributeName>
        static constexpr auto FindAttribute()
        {
            constexpr auto AttributeIndex = FindFirstIndex(AttributeTypes,
                [](auto attribute) constexpr -> bool
                {
                    using AttributeType = decltype(attribute);
                    return AttributeType::Name == AttributeName;
                });

            return Attributes.template Get<AttributeIndex>();
        }
    };

    template<typename ReflectedType>
    struct StaticTypeInfo
    {
        using Type = ReflectedType;
        static constexpr auto TypeInfo = Detail::TypeInfo<Type>{};
        using BaseType = typename decltype(TypeInfo)::BaseType;

        static constexpr bool Reflected = TypeInfo.Reflected;
        static constexpr std::string_view Name = TypeInfo.Name;
        static constexpr TypeIdentifier Identifier =
            Common::StringHash<TypeIdentifier>(TypeInfo.Name);
        static constexpr auto Attributes =
            Detail::MakeAttributeDescriptionWithInstanceList<Type>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());
        static constexpr auto AttributeTypes =
            Detail::MakeAttributeDescriptionWithoutInstanceList<Type>(
                TypeInfo.Attributes, std::make_index_sequence<TypeInfo.Attributes.Count>());
        static constexpr auto Members = Detail::MakeMemberDescriptionList<Type>(
                std::make_index_sequence<TypeInfo.Members.Count>());
        static constexpr auto Fields =
            Detail::FilterMemberDescriptionList<MemberKind::Field>(Members);
        static constexpr auto Methods =
            Detail::FilterMemberDescriptionList<MemberKind::Method>(Members);

        static constexpr bool IsNullType()
        {
            return std::is_same<Type, NullType>::value;
        }

        static constexpr bool IsConstructible()
        {
            return std::is_constructible<Type>::value;
        }

        static constexpr bool HasBaseType()
        {
            return !std::is_same<BaseType, NullType>::value;
        }

        static constexpr StaticTypeInfo<BaseType> GetBaseType()
        {
            return {};
        }

        template<typename OtherType>
        static constexpr bool IsType()
        {
            return std::is_same<Type, OtherType>::value;
        }

        template<typename OtherType>
        static constexpr bool IsBaseOf()
        {
            return std::is_same<Type, typename StaticTypeInfo<OtherType>::BaseType>::value;
        }

        template<typename OtherType>
        static constexpr bool IsDerivedFrom()
        {
            return std::is_same<BaseType, OtherType>::value;
        }

        static constexpr bool HasAttributes()
        {
            return Attributes.Count > 0;
        }

        template<typename AttributeType>
        static constexpr bool HasAttribute()
        {
            return TypeInfo.Attributes.template Contains<AttributeType>();
        }

        template<std::size_t AttributeIndex>
        static constexpr auto Attribute()
        {
            static_assert(Attributes.Count > AttributeIndex, "Out of bounds attribute index!");
            return Attributes.template Get<AttributeIndex>();
        }

        template<auto& AttributeName>
        static constexpr auto FindAttribute()
        {
            constexpr auto AttributeIndex = FindFirstIndex(AttributeTypes,
                [](auto attribute) constexpr -> bool
                {
                    using AttributeType = decltype(attribute);
                    return AttributeType::Name == AttributeName;
                });

            return Attributes.template Get<AttributeIndex>();
        }

        static constexpr bool HasMembers()
        {
            return Members.Count > 0;
        }

        template<std::size_t MemberIndex>
        static constexpr auto Member()
        {
            static_assert(Members.Count > MemberIndex, "Out of bounds member index!");
            return Members.template Get<MemberIndex>();
        }

        template<auto& MemberName>
        static constexpr auto FindMember()
        {
            return FindOne(Members, [](auto member) constexpr -> bool
            {
                using MemberType = decltype(member);
                return MemberType::Name == MemberName;
            });
        }
    };

    template<typename ReflectedType>
    using DecayedStaticTypeInfo =
        StaticTypeInfo<std::decay_t<std::remove_pointer_t<ReflectedType>>>;
}

