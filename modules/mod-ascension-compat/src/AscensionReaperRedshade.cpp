/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
//
// The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has transformed
// into Thresh!". Casting Thresh then applies Bloodshatter (Dummy) 525299 for the second step. Both
// carry Ascension aura 337 with the replacement in EffectMiscValue, which is what the client reads
// to redraw the button, so the client half of the transform needs nothing from the server.
//
// The server half is that the cast has to be legal. CMSG_CAST_SPELL is refused outright for a
// spell the caster does not have active - "cheater? kick? ban?" - so the redrawn button spent a
// global cooldown and nothing else happened. A Reaper never learns Thresh or Bloodshatter: they
// exist only as what Redshade turns Reap into, so owning the talent is what puts them in the
// spellbook, once, for as long as the talent is taken.
//
// Nothing is sent to the client, because everything that can be sent is announced in chat on this
// client. Player::learnSpell answers a temporary learn with SMSG_LEARNED_SPELL, which prints "You
// have learned a new ability: Thresh"; SendInitialSpells prints one line per ability the client
// did not already have; and SMSG_SUPERCEDED_SPELL, which is what
// Player::SetTemporarySpellReplacement sends to move a button, prints the same line for whichever
// spell it names - including the rank of Reap it hands the button back to. Player::addSpell with
// learnFromSkill set is the one way in that says nothing at all.
//
// spell_ascension_reaper_redshade_reap covers the other direction: a client that still sends the
// rank of Reap on the bar gets the replacement cast for it.

#include "Player.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"

namespace
{
constexpr uint32 SPELL_THRESH_DUMMY = 525058;
constexpr uint32 SPELL_THRESH = 505170;
constexpr uint32 SPELL_BLOODSHATTER_DUMMY = 525299;
constexpr uint32 SPELL_BLOODSHATTER = 505326;

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
            if (!player->HasActiveSpell(spellId))
                // temporary, so it is never written to the character; learnFromSkill, so
                // _addSpell does not announce it, and addSpell sends no packet of its own.
                player->addSpell(spellId, player->GetActiveSpecMask(), true, true, true);
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        // onlyTemporary, so a copy the character owns in its own right stays. This runs when the
        // talent itself goes, not between transforms.
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

// The cast itself, on every rank of Reap, for the client that still sends the rank on the bar.
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
    RegisterSpellScript(aura_ascension_reaper_redshade_spells);
    RegisterSpellScript(spell_ascension_reaper_redshade_reap);
}
