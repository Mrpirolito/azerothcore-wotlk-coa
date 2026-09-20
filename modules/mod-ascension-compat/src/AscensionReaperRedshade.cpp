/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
//
// The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has transformed
// into Thresh!". The aura is a plain SPELL_AURA_DUMMY, so nothing acted on it: the buff appeared
// and the button still said Reap. Casting Thresh then applies Bloodshatter (Dummy) 525299 for the
// second step, with the same problem.
//
// Two halves answer it, and neither one touches the spellbook.
//
// The button is redrawn with SMSG_SUPERCEDED_SPELL, the packet the client already reads to swap an
// action button when one spell replaces another. Player::SetTemporarySpellReplacement sends the
// same packet and was the first thing tried, but it only accepts a replacement the character owns,
// and a Reaper never learns Thresh or Bloodshatter: they exist only as what Redshade turns Reap
// into. Teaching them made the client announce "You have learned a new ability" and its unlearn in
// chat on every single transform, and the learn hooks re-ran the spec synchronisation, which
// announced every rank of Reap along with them.
//
// The cast is answered by spell_ascension_reaper_redshade_reap on the ranks of Reap, so the right
// ability goes out whether the client sends the replacement it was told about or the rank it still
// holds on the bar.
//
// A character owns several ranks of Reap at once, because the spec hands out the lower ranks as
// well, and the button holds whichever rank the client drew - rank 8 for a Reaper at 80 - so every
// owned rank is swapped. Swapping only the first put Thresh behind rank 1 while the bar still said
// Reap.
//
// Not aura 337, one of the Ascension auras the core leaves at nullptr: its 61 rows do not agree on
// what the fields mean, and implementing it against contradictory data would have put the other
// sixty at risk for one ability.

#include "Opcodes.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <array>

namespace
{
constexpr uint32 SPELL_THRESH_DUMMY = 525058;
constexpr uint32 SPELL_THRESH = 505170;
constexpr uint32 SPELL_BLOODSHATTER_DUMMY = 525299;
constexpr uint32 SPELL_BLOODSHATTER = 505326;

constexpr std::array<uint32, 9> ReapRanks = { 354319, 500357, 504056, 504057, 504058, 504557,
    505151, 573302, 573303 };

void SendButtonSwap(Player* player, uint32 from, uint32 to)
{
    if (!player->GetSession())
        return;

    WorldPacket packet(SMSG_SUPERCEDED_SPELL, 8);
    packet << uint32(from);
    packet << uint32(to);
    player->GetSession()->SendPacket(&packet);
}

// Each step of the transform, on the buff that announces it.
class aura_ascension_reaper_redshade_transform : public AuraScript
{
    PrepareAuraScript(aura_ascension_reaper_redshade_transform);

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
        if (Player* player = GetTarget()->ToPlayer())
            for (uint32 rank : ReapRanks)
                if (player->HasActiveSpell(rank))
                    SendButtonSwap(player, rank, Replacement());
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        uint32 const other = GetId() == SPELL_BLOODSHATTER_DUMMY ? SPELL_THRESH_DUMMY
            : SPELL_BLOODSHATTER_DUMMY;
        // The second step replaces the same buttons and is applied before this one is removed, so
        // a button is only given back once no step is left to own it.
        if (!player || player->HasAura(other))
            return;

        for (uint32 rank : ReapRanks)
            if (player->HasActiveSpell(rank))
                SendButtonSwap(player, Replacement(), rank);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_ascension_reaper_redshade_transform::Apply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_ascension_reaper_redshade_transform::Remove,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// The cast itself, on every rank of Reap.
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
    RegisterSpellScript(aura_ascension_reaper_redshade_transform);
    RegisterSpellScript(spell_ascension_reaper_redshade_reap);
}
