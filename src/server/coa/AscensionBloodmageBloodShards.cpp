/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

#include "AscensionPooledVitality.h"
#include "Player.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "SpellScript.h"

#include <algorithm>
#include <limits>

namespace
{
enum BloodShardSpells : uint32
{
    SPELL_BLOOD_SHARDS = 804849,
    SPELL_BATTLEWEAVER = 801963,
    SPELL_INHUMANE = 807488,
    SPELL_EVERLASTING_HUNT = 804686,
    SPELL_EVERLASTING_HUNT_COOLDOWN = 802497,
    SPELL_SHARD_COUNTER = 505366,
    SPELL_SHARD_VISUAL_FIRST = 505349,
    SPELL_SHARD_DAMAGE = 504115,
    SPELL_VEINBURST = 504260,
    SPELL_AORTIC_ASSAULT_HIT = 806502,
    SPELL_CURSED_FORM_REQUIREMENT = 525031,
    SPELL_CURSED_FORM_REQUIREMENT_2 = 524861
};

constexpr uint8 MAX_SHARDS = 8;

Player* Bloodmage(Unit* unit)
{
    Player* player = unit ? unit->ToPlayer() : nullptr;
    return player && player->getClass() == CLASS_SON_OF_ARUGAL && player->IsAlive() && player->IsInWorld() ? player : nullptr;
}

uint8 Shards(Player* player)
{
    Aura const* counter = player->GetAura(SPELL_SHARD_COUNTER, player->GetGUID());
    return counter ? counter->GetStackAmount() : 0;
}

void ClearShards(Player* player)
{
    player->RemoveAurasDueToSpell(SPELL_SHARD_COUNTER, player->GetGUID());
    for (uint8 i = 0; i < MAX_SHARDS; ++i)
        player->RemoveAurasDueToSpell(SPELL_SHARD_VISUAL_FIRST + i, player->GetGUID());
}

void GenerateShards(Player* player, uint8 count)
{
    uint8 total = std::min<uint8>(MAX_SHARDS, Shards(player) + count);
    if (!total)
        return;
    Aura* counter = player->GetAura(SPELL_SHARD_COUNTER, player->GetGUID());
    if (!counter)
        counter = player->AddAura(SPELL_SHARD_COUNTER, player);
    if (!counter)
        return;
    counter->SetStackAmount(total);
    Aura* first = nullptr;
    for (uint8 i = 0; i < total; ++i)
    {
        Aura* visual = player->GetAura(SPELL_SHARD_VISUAL_FIRST + i, player->GetGUID());
        if (!visual)
            visual = player->AddAura(SPELL_SHARD_VISUAL_FIRST + i, player);
        if (visual)
            visual->RefreshDuration();
        if (!i)
            first = visual;
    }
    if (first)
    {
        counter->SetMaxDuration(first->GetMaxDuration());
        counter->SetDuration(first->GetDuration());
    }
}

void LaunchShard(Player* player, Unit* target, float apCoefficient, float spCoefficient)
{
    SpellInfo const* shard = sSpellMgr->GetSpellInfo(SPELL_SHARD_DAMAGE);
    if (!shard || !target || !target->IsAlive())
        return;
    float amount = float(shard->Effects[EFFECT_0].CalcValue(player)) +
        player->GetTotalAttackPowerValue(BASE_ATTACK) * apCoefficient +
        float(player->SpellBaseDamageBonusDone(SPELL_SCHOOL_MASK_SHADOW)) * spCoefficient;
    player->CastCustomSpell(SPELL_SHARD_DAMAGE, SPELLVALUE_BASE_POINT0,
        int32(std::clamp(amount, 1.0f, float(std::numeric_limits<int32>::max() / 2))), target, TRIGGERED_FULL_MASK);
}

bool IsCursedFormAbility(SpellInfo const* info)
{
    return info->CasterAuraSpell == SPELL_CURSED_FORM_REQUIREMENT ||
        info->CasterAuraSpell == SPELL_CURSED_FORM_REQUIREMENT_2;
}

bool IsVeinburst(uint32 id)
{
    return id == SPELL_VEINBURST || sSpellMgr->GetFirstSpellInChain(id) == SPELL_VEINBURST;
}

class bloodmage_blood_shards : public AllSpellScript
{
public:
    bloodmage_blood_shards() : AllSpellScript("bloodmage_blood_shards",
        {ALLSPELLHOOK_ON_CAST, ALLSPELLHOOK_ON_HIT_RESULT}) { }

    void OnSpellCast(Spell* spell, Unit* caster, SpellInfo const* info, bool) override
    {
        Player* player = Bloodmage(caster);
        if (!player || spell->IsTriggered() || info->SpellFamilyName != 26 || !IsCursedFormAbility(info))
            return;
        if (SpellInfo const* talent = sSpellMgr->GetSpellInfo(SPELL_BATTLEWEAVER);
            talent && player->HasAura(SPELL_BATTLEWEAVER) && roll_chance_i(int32(talent->ProcChance)))
            GenerateShards(player, 1);
    }

    void OnSpellHitResult(Spell* spell, Unit* target, uint8 miss, uint32 damage, uint32, bool critical) override
    {
        Player* player = Bloodmage(spell->GetCaster());
        if (!player || !target || target == player || miss != SPELL_MISS_NONE || !damage ||
            !player->IsValidAttackTarget(target))
            return;
        SpellInfo const* info = spell->GetSpellInfo();
        if (info->Id == SPELL_SHARD_DAMAGE)
        {
            if (player->HasAura(SPELL_EVERLASTING_HUNT))
                player->CastSpell(player, SPELL_EVERLASTING_HUNT_COOLDOWN, true);
            return;
        }
        if (IsVeinburst(info->Id))
        {
            for (uint8 shards = Shards(player); shards; --shards)
                LaunchShard(player, target, 0.05f, 0.1f);
            ClearShards(player);
            return;
        }
        if (info->Id == SPELL_AORTIC_ASSAULT_HIT && player->HasAura(SPELL_INHUMANE))
            LaunchShard(player, target, 0.15f, 0.35f);
        if (player->HasAura(SPELL_BATTLEWEAVER) &&
            AscensionBloodmage::GetEmpowerment(info->Id) == AscensionBloodmage::Bloodbolt)
            LaunchShard(player, target, 0.05f, 0.1f);
        if (player->HasAura(SPELL_BLOOD_SHARDS) && (info->GetSchoolMask() & SPELL_SCHOOL_MASK_SHADOW))
            GenerateShards(player, critical ? 2 : 1);
    }
};

class aura_ascension_bloodmage_blood_shard_expiry : public AuraScript
{
    PrepareAuraScript(aura_ascension_bloodmage_blood_shard_expiry);

    void Expire(AuraEffect const*, AuraEffectHandleModes)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;
        if (Player* player = GetTarget()->ToPlayer())
            ClearShards(player);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(aura_ascension_bloodmage_blood_shard_expiry::Expire, EFFECT_0,
            SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};
}

void AddSC_AscensionBloodmageBloodShards()
{
    new bloodmage_blood_shards();
    RegisterSpellScript(aura_ascension_bloodmage_blood_shard_expiry);
}
