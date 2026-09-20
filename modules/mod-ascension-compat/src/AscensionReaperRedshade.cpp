/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
//
// The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has transformed
// into Thresh!". The aura is a plain SPELL_AURA_DUMMY, so nothing acted on it: the buff appeared
// and the button still said Reap. Casting Thresh then applies Bloodshatter (Dummy) 525299 for the
// second step, with the same problem.
//
// Both are answered with Player::SetTemporarySpellReplacement, the same mechanism Hemostasis and
// the spec drivers use: the button is redrawn through SMSG_SUPERCEDED_SPELL and the cast handler
// sends the replacement instead, so the player sees what the tooltip promised rather than a Reap
// that quietly does something else. The replacement is only in the spellbook while the buff is,
// and it is taught only when it is not already owned, so a character who learned Thresh on their
// own keeps it when the buff falls off.
//
// Not aura 337, one of the Ascension auras the core leaves at nullptr: its 61 rows do not agree on
// what the fields mean, and implementing it against contradictory data would have put the other
// sixty at risk for one ability.

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include <array>

namespace
{
constexpr uint32 SPELL_THRESH_DUMMY = 525058;
constexpr uint32 SPELL_THRESH = 505170;
constexpr uint32 SPELL_BLOODSHATTER_DUMMY = 525299;
constexpr uint32 SPELL_BLOODSHATTER = 505326;

// Every rank of Reap. Whichever one the character owns is the button the buff replaces.
constexpr std::array<uint32, 9> ReapRanks = { 354319, 500357, 504056, 504057, 504058, 504557,
    505151, 573302, 573303 };

uint32 OwnedReapRank(Player* player)
{
    for (uint32 rank : ReapRanks)
        if (player->HasActiveSpell(rank))
            return rank;

    return 0;
}

class aura_ascension_reaper_redshade_transform : public AuraScript
{
    PrepareAuraScript(aura_ascension_reaper_redshade_transform);

    uint32 _button = 0;
    bool _taught = false;

    uint32 Replacement() const
    {
        return GetId() == SPELL_BLOODSHATTER_DUMMY ? SPELL_BLOODSHATTER : SPELL_THRESH;
    }

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_THRESH, SPELL_BLOODSHATTER });
    }

    void Apply(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        uint32 const button = OwnedReapRank(player);
        if (!button)
            return;

        uint32 const replacement = Replacement();
        // Preserve independent permanent or other-spec ownership, as the other drivers do.
        if (player->GetSpellMap().find(replacement) == player->GetSpellMap().end())
        {
            player->learnSpell(replacement, true);
            _taught = true;
        }

        player->SetTemporarySpellReplacement(button, replacement);
        if (player->GetTemporarySpellReplacement(button) == replacement)
            _button = button;
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !_button)
            return;

        // The second step replaces the same button, and its aura is applied before this one is
        // removed. Only give the button back if it is still showing what this buff put there.
        if (player->GetTemporarySpellReplacement(_button) == Replacement())
            player->SetTemporarySpellReplacement(_button, 0);

        if (_taught)
            player->removeSpell(Replacement(), SPEC_MASK_ALL, true);

        _button = 0;
        _taught = false;
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_ascension_reaper_redshade_transform::Apply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_ascension_reaper_redshade_transform::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};
}

void AddSC_AscensionReaperRedshade()
{
    RegisterSpellScript(aura_ascension_reaper_redshade_transform);
}
