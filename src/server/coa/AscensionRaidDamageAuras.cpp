/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "Unit.h"

#include <algorithm>
#include <limits>

namespace
{
enum class RaidAuraScaling
{
    AttackPower,
    RangedAttackPower,
    ShadowSpellPower,
    ArcaneSpellPower
};

struct RaidDamageAura
{
    uint32 Aura;
    RaidAuraScaling Scaling;
    float Coefficient;
};

constexpr RaidDamageAura RaidDamageAuras[] =
{
    {560530, RaidAuraScaling::AttackPower, 0.35f},
    {573241, RaidAuraScaling::AttackPower, 0.35f},
    {537248, RaidAuraScaling::RangedAttackPower, 0.35f},
    {520839, RaidAuraScaling::ShadowSpellPower, 0.6f},
    {520929, RaidAuraScaling::ArcaneSpellPower, 0.7f}
};

RaidDamageAura const* FindRaidDamageAura(uint32 aura)
{
    auto itr = std::find_if(std::begin(RaidDamageAuras), std::end(RaidDamageAuras),
        [aura](RaidDamageAura const& entry) { return entry.Aura == aura; });
    return itr == std::end(RaidDamageAuras) ? nullptr : &*itr;
}

float ScalingValue(Unit* source, RaidAuraScaling scaling)
{
    switch (scaling)
    {
        case RaidAuraScaling::AttackPower: return source->GetTotalAttackPowerValue(BASE_ATTACK);
        case RaidAuraScaling::RangedAttackPower: return source->GetTotalAttackPowerValue(RANGED_ATTACK);
        case RaidAuraScaling::ShadowSpellPower: return float(source->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW));
        case RaidAuraScaling::ArcaneSpellPower: return float(source->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_ARCANE));
    }
    return 0.0f;
}

class aura_ascension_raid_damage_aura : public AuraScript
{
    PrepareAuraScript(aura_ascension_raid_damage_aura);

    void Amount(AuraEffect const*, int32& amount, bool&)
    {
        RaidDamageAura const* entry = FindRaidDamageAura(GetId());
        if (Unit* caster = GetCaster(); entry && caster)
            amount = int32(std::clamp(ScalingValue(caster->GetCharmerOrOwnerOrSelf(), entry->Scaling) * entry->Coefficient,
                0.0f, float(std::numeric_limits<int32>::max() / 2)));
    }

    void Proc(AuraEffect const* effect, ProcEventInfo& event)
    {
        PreventDefaultAction();
        Unit* ally = GetTarget();
        if (Unit* victim = event.GetActionTarget(); victim && victim != ally && effect->GetAmount() > 0)
            ally->CastCustomSpell(effect->GetSpellInfo()->Effects[EFFECT_0].TriggerSpell, SPELLVALUE_BASE_POINT0,
                effect->GetAmount(), victim, TRIGGERED_FULL_MASK, nullptr, effect, ally->GetGUID());
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(aura_ascension_raid_damage_aura::Amount, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE);
        OnEffectProc += AuraEffectProcFn(aura_ascension_raid_damage_aura::Proc, EFFECT_0,
            SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE);
    }
};
}

void AddSC_AscensionRaidDamageAuras()
{
    RegisterSpellScript(aura_ascension_raid_damage_aura);
}
