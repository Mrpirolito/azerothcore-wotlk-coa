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
// hit it, takes their GUID as owner, and runs. What keeps the caster out of the fight is that the
// visage never fights back: passive, no combat movement, an empty AttackStart.
//
// Haunt is castable only from Stealth or Shadowform, which its ShapeshiftMask 0x28000000 says and
// the Reaper enters with Underwalk 800797.

#include "Creature.h"
#include "Random.h"
#include "CreatureAI.h"
#include "MotionMaster.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include <cmath>

namespace
{
constexpr uint32 NPC_REAPER_HAUNT_VISAGE = 990020;

// "Clone Me!", the generic SPELL_AURA_CLONE_CASTER carrier Mirror Image also uses.
constexpr uint32 SPELL_CLONE_CASTER = 45204;

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

            // SetDisplayId alone gives the base race model with no skin, hair or gear, which
            // renders as a white silhouette. "Clone Me!" carries SPELL_AURA_CLONE_CASTER, the
            // same aura Mirror Image uses, so the client draws the visage as the caster itself.
            me->SetDisplayId(player->GetDisplayId());
            player->CastSpell(me, SPELL_CLONE_CASTER, true);
            me->SetFaction(player->GetFaction());
            me->SetLevel(player->GetLevel());
            me->SetMaxHealth(std::max(5u, uint32(player->GetLevel()) * 10));
            me->SetHealth(me->GetMaxHealth());

            // The owner link is what lets the caster see it as theirs; what keeps them out of the
            // fight is that the visage never fights back. It is passive, it does not move to
            // engage, and AttackStart below is deliberately empty, so nothing ever asks the owner
            // to assist.
            me->SetOwnerGUID(player->GetGUID());
            me->SetCreatorGUID(player->GetGUID());
            me->SetReactState(REACT_PASSIVE);
            me->SetCombatMovement(false);

            // Any direction, not the one the caster happens to face: the visage is a decoy, and
            // one that always breaks the same way is one an enemy learns to ignore.
            float const angle = frand(0.0f, 2.0f * float(M_PI));
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
