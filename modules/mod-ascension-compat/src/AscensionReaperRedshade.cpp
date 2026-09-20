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
// that quietly does something else.
//
// That needs the replacement in the spellbook, and a Reaper never learns Thresh or Bloodshatter:
// they exist only as what Redshade turns Reap into. Owning the talent is therefore what puts them
// there, not each buff - the client announces every learned and forgotten spell in chat, so
// teaching them per transform filled the log with "You have learned" and "You have unlearned" for
// every single Reap. It also broke the second step: Thresh's own cast is what applies the
// Bloodshatter buff, so taking Thresh out of the book as its buff fell off removed the aura Thresh
// had caused along with it.
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
constexpr uint32 SPELL_REDSHADE = 524735;
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

// Taking the talent is what owns the two transformed abilities.
class aura_ascension_reaper_redshade_spells : public AuraScript
{
    PrepareAuraScript(aura_ascension_reaper_redshade_spells);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_THRESH, SPELL_BLOODSHATTER });
    }

    void Apply(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        for (uint32 spellId : { SPELL_THRESH, SPELL_BLOODSHATTER })
            // Preserve independent permanent or other-spec ownership, as the other drivers do.
            if (player->GetSpellMap().find(spellId) == player->GetSpellMap().end())
                player->learnSpell(spellId, true);
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        // onlyTemporary, so a copy the character owns in its own right stays.
        for (uint32 spellId : { SPELL_THRESH, SPELL_BLOODSHATTER })
            player->removeSpell(spellId, SPEC_MASK_ALL, true);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_ascension_reaper_redshade_spells::Apply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_ascension_reaper_redshade_spells::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Each step of the transform, on the buff that announces it.
class aura_ascension_reaper_redshade_transform : public AuraScript
{
    PrepareAuraScript(aura_ascension_reaper_redshade_transform);

    std::vector<uint32> _buttons;

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
        for (uint32 rank : ReapRanks)
        {
            if (!player->HasActiveSpell(rank))
                continue;

            player->SetTemporarySpellReplacement(rank, replacement);
            if (player->GetTemporarySpellReplacement(rank) == replacement)
                _buttons.push_back(rank);
        }
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        // The second step replaces the same buttons, and its aura is applied before this one is
        // removed. Only give a button back if it still shows what this buff put there.
        for (uint32 rank : _buttons)
            if (player->GetTemporarySpellReplacement(rank) == Replacement())
                player->SetTemporarySpellReplacement(rank, 0);

        _buttons.clear();
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
    RegisterSpellScript(aura_ascension_reaper_redshade_spells);
    RegisterSpellScript(aura_ascension_reaper_redshade_transform);
}
