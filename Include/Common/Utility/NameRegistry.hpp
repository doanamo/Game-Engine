/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

/*
    Name Registry

    Registry of names for looking them up by hash identifier.
    This should generally be disabled in shipped game.
*/

namespace Common
{
    class Name;

    class NameRegistry : public NonCopyable
    {
    public:
        using HashType = uint32_t;

        NameRegistry();
        ~NameRegistry();

        static NameRegistry& GetInstance();

#ifdef NAME_REGISTRY_ENABLED
    public:
        void Register(std::string_view string);
        std::string_view Lookup(HashType hash);

        bool IsRegistered(std::string_view string);
        bool IsRegistered(HashType hash);

    private:
        friend Name;
        void Register(const Name& name, std::string_view string);
        std::unordered_map<HashType, std::string> m_registry;
#endif
    };
}
