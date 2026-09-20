/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
//
// The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has transformed
// into Thresh!". The aura is a plain SPELL_AURA_DUMMY, so nothing acted on it: the buff appeared
// and Reap stayed Reap. Casting Thresh then applies Bloodshatter (Dummy) 525299 for the second
// step, which had the same problem.
//
// Both are answered here rather than in the DBC. The alternative is aura 337, one of the Ascension
// auras the core leaves at nullptr, and its 61 rows do not agree on what the fields mean: "Thunder
// Blast!" reads as "replace the spell in EffectMiscValue with the one in EffectBasePoints", while
// 525299 carries a spell in EffectMiscValue and nothing in EffectBasePoints. Implementing an aura
// against contradictory data is worse than implementing this one ability against its tooltip.

#include "Player.h"
#include "ScriptMgr.h"
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

class spell_ascension_reaper_redshade_reap : public SpellScript
{
    PrepareSpellScript(spell_ascension_reaper_redshade_reap);

    SpellCastResult CheckCast()
    {
        Unit* caster = GetCaster();
        if (!caster || !caster->IsPlayer())
            return SPELL_CAST_OK;

        // Bloodshatter is the second step, so it wins while both are up.
        uint32 const replacement = caster->HasAura(SPELL_BLOODSHATTER_DUMMY) ? SPELL_BLOODSHATTER
            : caster->HasAura(SPELL_THRESH_DUMMY) ? SPELL_THRESH : 0;
        if (!replacement)
            return SPELL_CAST_OK;

        Unit* target = GetExplTargetUnit();
        caster->CastSpell(target ? target : caster, replacement, false);

        // The replacement is the cast the player asked for, so Reap must not also go out and must
        // not report a failure they did not cause.
        return SPELL_FAILED_DONT_REPORT;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_ascension_reaper_redshade_reap::CheckCast);
    }
};
}

void AddSC_AscensionReaperRedshade()
{
    RegisterSpellScript(spell_ascension_reaper_redshade_reap);
}
