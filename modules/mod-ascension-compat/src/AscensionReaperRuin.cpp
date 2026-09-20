/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Ruin marks what Shudder Scythe hits.
//
// The talent 805198 is a SPELL_AURA_PROC_TRIGGER_SPELL for the debuff 805199, and spell_proc
// gives it the flags its record omits. That is enough for the damaging rank 801322 cast on its
// own, and it is not enough for the way a player actually uses the ability: the button casts the
// transform 572382, whose aura triggers 801322 every 100 ms. A spell cast by a periodic aura tick
// does not reach the proc system the way a cast does, so the mark never landed in play.
//
// Rather than widen the proc flags until something sticks, the damaging rank applies the mark
// itself when its caster owns the talent. That is one place, it reads the way the tooltip does,
// and it covers both routes.

#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "Unit.h"

namespace
{
constexpr uint32 SPELL_RUIN_TALENT = 805198;
constexpr uint32 SPELL_RUIN = 805199;

class spell_ascension_reaper_ruin_mark : public SpellScript
{
    PrepareSpellScript(spell_ascension_reaper_ruin_mark);

    void MarkTarget(SpellEffIndex /*index*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!caster || !target || !caster->HasAura(SPELL_RUIN_TALENT))
            return;

        caster->CastSpell(target, SPELL_RUIN, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_ascension_reaper_ruin_mark::MarkTarget,
            EFFECT_0, SPELL_EFFECT_ANY);
    }
};
}

void AddSC_AscensionReaperRuin()
{
    RegisterSpellScript(spell_ascension_reaper_ruin_mark);
}
