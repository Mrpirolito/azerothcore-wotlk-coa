/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Haunt (573425) summons a visage of the Reaper that runs away to be attacked in their place.
//
// It summoned creature 840000, the Witch Doctor's Mirage, whose script starts with
//
//     if (!player || player->getClass() != CLASS_WITCH_DOCTOR)
//         return;
//
// so for a Reaper nothing happened at all: no copied appearance, no faction, no owner, no
// movement. The decoy stood where it was cast as a neutral level one creature.
//
// This is the same idea under the Reaper's own entry, so neither class has to know about the
// other. The visage copies the caster's appearance, takes their faction and level so enemies can
// hit it, holds no owner GUID so the caster is not pulled into its fights, and runs.

#include "Creature.h"
#include "CreatureAI.h"
#include "MotionMaster.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"

namespace
{
constexpr uint32 NPC_REAPER_HAUNT_VISAGE = 990020;

// Far enough that the visage is clearly leaving, close enough to stay in the fight it is stealing.
constexpr float FleeDistance = 40.0f;

class npc_ascension_reaper_haunt : public CreatureScript
{
public:
    npc_ascension_reaper_haunt() : CreatureScript("npc_ascension_reaper_haunt") { }

    struct npc_ascension_reaper_hauntAI : public CreatureAI
    {
        npc_ascension_reaper_hauntAI(Creature* creature) : CreatureAI(creature) { }

        void IsSummonedBy(WorldObject* summoner) override
        {
            Player* player = summoner ? summoner->ToPlayer() : nullptr;
            if (!player)
                return;

            me->SetDisplayId(player->GetDisplayId());
            me->SetFaction(player->GetFaction());
            me->SetLevel(player->GetLevel());
            me->SetMaxHealth(std::max(5u, uint32(player->GetLevel()) * 10));
            me->SetHealth(me->GetMaxHealth());

            // Passive and with no owner: the visage never fights, and the caster is not dragged
            // into combat when something turns on it. That is the whole point of the ability.
            me->SetReactState(REACT_PASSIVE);
            me->SetCombatMovement(false);
            me->SetCreatorGUID(player->GetGUID());

            // Straight out from the caster, keeping the direction they were facing, so the visage
            // reads as the Reaper breaking away rather than wandering.
            float const angle = player->GetOrientation();
            float x = player->GetPositionX() + FleeDistance * std::cos(angle);
            float y = player->GetPositionY() + FleeDistance * std::sin(angle);
            float z = player->GetPositionZ();
            me->UpdateAllowedPositionZ(x, y, z);
            me->GetMotionMaster()->MovePoint(0, x, y, z);
        }

        // Nothing it is hit by should make it stop and fight back.
        void AttackStart(Unit* /*who*/) override { }
        void EnterEvadeMode(EvadeReason /*why*/) override { }
        void UpdateAI(uint32 /*diff*/) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ascension_reaper_hauntAI(creature);
    }
};
}

void AddSC_AscensionReaperHaunt()
{
    new npc_ascension_reaper_haunt();
}
