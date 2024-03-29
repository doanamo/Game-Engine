/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include <unordered_set>
#include <unordered_map>
#include <Common/Events/EventReceiver.hpp>
#include "Game/GameSystem.hpp"
#include "Game/EntityHandle.hpp"

/*
    Identity System

    Manages entity names and groups that they belong to and allows them to be queried by them.
*/

namespace Game
{
    class EntitySystem;
    class GameInstance;

    class IdentitySystem final : public GameSystem
    {
        REFLECTION_ENABLE(IdentitySystem, GameSystem)

    public:
        using EntityNameLookup = std::unordered_map<EntityHandle, std::string>;
        using NameEntityLookup = std::unordered_map<std::string, EntityHandle>;

        using EntityGroupsSet = std::unordered_set<std::string>;
        using GroupEntitiesSet = std::unordered_set<EntityHandle>;

        using EntityGroupsLookup = std::unordered_map<EntityHandle, EntityGroupsSet>;
        using GroupEntiriesLookup = std::unordered_map<std::string, GroupEntitiesSet>;

        enum class NamingErrors
        {
            InvalidEntity,
            ReservedName,
        };

        enum class GroupingErrors
        {
            InvalidEntity,
            InvalidGroup,
        };

        using NamingResult = Common::Result<void, NamingErrors>;
        using GroupingResult = Common::Result<void, GroupingErrors>;

        enum class LookupErrors
        {
            InvalidEntity,
            EntityNotFound,
            NameNotFound,
            GroupNotFound,
        };

        template<typename Type>
        using LookupResult = Common::Result<Type, LookupErrors>;

    public:
        IdentitySystem();
        ~IdentitySystem() override;

        NamingResult SetEntityName(EntityHandle entity, std::string name, bool force = false);
        LookupResult<EntityHandle> GetEntityByName(std::string name) const;
        LookupResult<std::string> GetEntityName(EntityHandle entity) const;
        std::size_t GetNamedEntityCount() const;

        GroupingResult SetEntityGroup(EntityHandle entity, std::string group);
        GroupingResult ClearEntityGroup(EntityHandle entity, std::string group);
        LookupResult<GroupEntitiesSet> GetEntitiesByGroup(std::string group) const;
        LookupResult<EntityGroupsSet> GetEntityGroups(EntityHandle entity) const;
        bool IsEntityInGroup(EntityHandle entity, std::string group) const;
        std::size_t GetEntityGroupCount(EntityHandle entity) const;

        std::size_t GetGroupedEntityCount() const
        {
            return m_entityGroupsLookup.size();
        }

        std::size_t GetGroupCount() const
        {
            return m_groupEntitiesLookup.size();
        }

    private:
        bool OnAttach(const GameSystemStorage& gameSystems) override;
        void OnEntityDestroyed(EntityHandle entity);

        void RegisterNamedEntity(const EntityHandle& entity, const std::string& name);
        void UnregisterNamedEntity(const EntityNameLookup::iterator entityNameIt);
        void UnregisterNamedEntity(const EntityHandle& entity);

        void RegisterGroupedEntity(const EntityHandle& entity, const std::string& group);
        void UnregisterGroupedEntity(const EntityHandle& entity, const std::string& group);
        void UnregisterGroupedEntity(const EntityHandle& entity);

        EntitySystem* m_entitySystem = nullptr;

        EntityNameLookup m_entityNameLookup;
        NameEntityLookup m_nameEntityLookup;

        EntityGroupsLookup m_entityGroupsLookup;
        GroupEntiriesLookup m_groupEntitiesLookup;

        Event::Receiver<void(EntityHandle)> m_entityDestroyReceiver;
    };
}

REFLECTION_TYPE(Game::IdentitySystem, Game::GameSystem)
