/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */


#include "Creature.h"
#include "LocalLevelScaling.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"
#include "WorldState.h"

namespace
{
enum LevelScalingToggle : uint32
{
    WorldStateCreatureScaling = 2100001,
    WorldStateQuestScaling = 2100002,

    StateUnset = 0,
    StateOff = 1,
    StateOn = 2,

    GossipTextScaling = 990010,

    ActionToggleScaling = GOSSIP_ACTION_INFO_DEF + 1
};

bool ResolveStored(uint32 worldStateId, bool configured)
{
    switch (sWorldState->getWorldState(worldStateId))
    {
        case StateOn:
            return true;
        case StateOff:
            return false;
        default:
            return configured;
    }
}

void Store(uint32 worldStateId, bool enabled)
{
    sWorldState->setWorldState(worldStateId, uint64(enabled ? StateOn : StateOff));
}

class npc_ascension_level_scaling_toggle : public CreatureScript
{
public:
    npc_ascension_level_scaling_toggle() : CreatureScript("npc_ascension_level_scaling_toggle") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        ClearGossipMenuFor(player);

        bool const scaling = LocalLevelScaling::CreatureEnabled.load(std::memory_order_relaxed);

        AddGossipItemFor(player, GOSSIP_ICON_CHAT,
            scaling ? "Level scaling is ON - turn it off." : "Level scaling is OFF - turn it on.",
            GOSSIP_SENDER_MAIN, ActionToggleScaling);

        SendGossipMenuFor(player, GossipTextScaling, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action) override
    {
        if (sender != GOSSIP_SENDER_MAIN)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        if (action != ActionToggleScaling)
        {
            CloseGossipMenuFor(player);
            return true;
        }

        bool const enabled = !LocalLevelScaling::CreatureEnabled.load(std::memory_order_relaxed);
        LocalLevelScaling::CreatureEnabled.store(enabled, std::memory_order_relaxed);
        LocalLevelScaling::QuestEnabled.store(enabled, std::memory_order_relaxed);
        Store(WorldStateCreatureScaling, enabled);
        Store(WorldStateQuestScaling, enabled);

        return OnGossipHello(player, creature);
    }
};

class AscensionLevelScalingToggleWorldScript : public WorldScript
{
public:
    AscensionLevelScalingToggleWorldScript()
        : WorldScript("AscensionLevelScalingToggleWorldScript", {WORLDHOOK_ON_STARTUP}) { }

    void OnStartup() override
    {
        LocalLevelScaling::CreatureEnabled.store(
            ResolveStored(WorldStateCreatureScaling,
                LocalLevelScaling::CreatureEnabled.load(std::memory_order_relaxed)),
            std::memory_order_relaxed);
        LocalLevelScaling::QuestEnabled.store(
            ResolveStored(WorldStateQuestScaling,
                LocalLevelScaling::QuestEnabled.load(std::memory_order_relaxed)),
            std::memory_order_relaxed);
    }
};
}

void AddSC_AscensionLevelScalingToggle()
{
    new npc_ascension_level_scaling_toggle();
    new AscensionLevelScalingToggleWorldScript();
}
