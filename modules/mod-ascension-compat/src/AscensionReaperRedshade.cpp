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
#include <vector>

namespace
{
constexpr uint32 SPELL_THRESH_DUMMY = 525058;
constexpr uint32 SPELL_THRESH = 505170;
constexpr uint32 SPELL_BLOODSHATTER_DUMMY = 525299;
constexpr uint32 SPELL_BLOODSHATTER = 505326;

// Every rank of Reap. A character owns several at once, because the spec hands out the lower
// ranks as well, and the button is whichever rank the client drew - rank 8 for a Reaper at 80.
// So every owned rank is swapped rather than the first one found: swapping only the first put
// Thresh behind rank 1 while the bar still held rank 8, which looks exactly like nothing
// happening.
constexpr std::array<uint32, 9> ReapRanks = { 354319, 500357, 504056, 504057, 504058, 504557,
    505151, 573302, 573303 };

class aura_ascension_reaper_redshade_transform : public AuraScript
{
    PrepareAuraScript(aura_ascension_reaper_redshade_transform);

    std::vector<uint32> _buttons;
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

        uint32 const replacement = Replacement();
        // Preserve independent permanent or other-spec ownership, as the other drivers do.
        if (player->GetSpellMap().find(replacement) == player->GetSpellMap().end())
        {
            player->learnSpell(replacement, true);
            _taught = true;
        }

        for (uint32 rank : ReapRanks)
        {
            if (!player->HasActiveSpell(rank))
                continue;

            player->SetTemporarySpellReplacement(rank, replacement);
            if (player->GetTemporarySpellReplacement(rank) == replacement)
                _buttons.push_back(rank);
        }

        // No button took it, so the spellbook copy is doing nothing either.
        if (_buttons.empty() && _taught)
        {
            player->removeSpell(replacement, SPEC_MASK_ALL, true);
            _taught = false;
        }
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || _buttons.empty())
            return;

        // The second step replaces the same buttons, and its aura is applied before this one is
        // removed. Only give a button back if it still shows what this buff put there.
        for (uint32 rank : _buttons)
            if (player->GetTemporarySpellReplacement(rank) == Replacement())
                player->SetTemporarySpellReplacement(rank, 0);

        // Thresh's own cast is what applies the Bloodshatter step, so taking Thresh out of the
        // spellbook while that step is still up removes the aura Thresh caused along with it, and
        // the button falls back to Reap one cast early. The copies go back only once neither step
        // is left. onlyTemporary keeps any independent ownership untouched.
        uint32 const other = GetId() == SPELL_BLOODSHATTER_DUMMY ? SPELL_THRESH_DUMMY
            : SPELL_BLOODSHATTER_DUMMY;
        if (!player->HasAura(other))
        {
            player->removeSpell(SPELL_THRESH, SPEC_MASK_ALL, true);
            player->removeSpell(SPELL_BLOODSHATTER, SPEC_MASK_ALL, true);
        }

        _buttons.clear();
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
