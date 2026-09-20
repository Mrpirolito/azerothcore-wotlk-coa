/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
//
// The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has transformed
// into Thresh!". The aura is a plain SPELL_AURA_DUMMY, so nothing acted on it: the buff appeared
// and the button still said Reap. Casting Thresh then applies Bloodshatter (Dummy) 525299 for the
// second step, with the same problem.
//
// Three things have to be true at once, and each one rules out the obvious shortcut.
//
// The server has to accept the cast. CMSG_CAST_SPELL is refused outright for a spell the caster
// does not have active - "cheater? kick? ban?" - so the button can be redrawn all it likes and
// nothing happens but a global cooldown. A Reaper never learns Thresh or Bloodshatter: they exist
// only as what Redshade turns Reap into, so owning the talent is what puts them in the spellbook.
//
// Learning them must not announce anything. Player::learnSpell answers a temporary learn with
// SMSG_LEARNED_SPELL, which the client prints as "You have learned a new ability: Thresh", and
// every transform would print it again. Player::addSpell with learnFromSkill set skips that
// packet, and one SendInitialSpells afterwards gives the client the spellbook it needs without a
// line of chat.
//
// The button is then swapped with SMSG_SUPERCEDED_SPELL, the packet the client already reads for
// this. It is silent only because the spell on the receiving end is one the client knows: sending
// it for a spell the client has never seen is what made it announce Thresh, and announce the ranks
// of Reap again on the way back.
//
// A character owns several ranks of Reap at once, because the spec hands out the lower ranks as
// well, so the swap addresses the highest rank the character has - the one the client draws on the
// bar - rather than all of them, which sent eight packets to move one button.
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
constexpr uint32 SPELL_REDSHADE = 524735;
constexpr uint32 SPELL_THRESH_DUMMY = 525058;
constexpr uint32 SPELL_THRESH = 505170;
constexpr uint32 SPELL_BLOODSHATTER_DUMMY = 525299;
constexpr uint32 SPELL_BLOODSHATTER = 505326;

// Lowest rank first, so the last match is the highest rank the character owns.
constexpr std::array<uint32, 9> ReapRanks = { 354319, 500357, 504056, 504057, 504058, 504557,
    505151, 573302, 573303 };

uint32 HighestOwnedReapRank(Player* player)
{
    uint32 highest = 0;
    for (uint32 rank : ReapRanks)
        if (player->HasActiveSpell(rank))
            highest = rank;

    return highest;
}

void SendButtonSwap(Player* player, uint32 from, uint32 to)
{
    if (!player->GetSession() || !from || !to)
        return;

    WorldPacket packet(SMSG_SUPERCEDED_SPELL, 8);
    packet << uint32(from);
    packet << uint32(to);
    player->GetSession()->SendPacket(&packet);
}

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

        bool added = false;
        for (uint32 spellId : { SPELL_THRESH, SPELL_BLOODSHATTER })
        {
            if (player->HasActiveSpell(spellId))
                continue;

            // temporary, so it is never written to the character; learnFromSkill, so _addSpell
            // does not announce it. addSpell sends no learned-spell packet of its own.
            player->addSpell(spellId, player->GetActiveSpecMask(), true, true, true);
            added = added || player->HasActiveSpell(spellId);
        }

        // One snapshot of the spellbook in place of two announcements.
        if (added)
            player->SendInitialSpells();
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        // onlyTemporary, so a copy the character owns in its own right stays. This runs when the
        // talent itself goes, not between transforms, so the one unlearn line it prints is the
        // truth: the abilities really are gone.
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

    uint32 _button = 0;

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

        _button = HighestOwnedReapRank(player);
        // The first step is on the bar, the second replaces the first.
        uint32 const previous = GetId() == SPELL_BLOODSHATTER_DUMMY && player->HasAura(SPELL_THRESH_DUMMY)
            ? SPELL_THRESH : _button;
        SendButtonSwap(player, previous, Replacement());
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        uint32 const other = GetId() == SPELL_BLOODSHATTER_DUMMY ? SPELL_THRESH_DUMMY
            : SPELL_BLOODSHATTER_DUMMY;
        // The second step owns the button while it lasts, so the first one does not take it back.
        if (player && !player->HasAura(other))
            SendButtonSwap(player, Replacement(), _button);

        _button = 0;
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(aura_ascension_reaper_redshade_transform::Apply,
            EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(aura_ascension_reaper_redshade_transform::Remove,
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
    RegisterSpellScript(aura_ascension_reaper_redshade_transform);
    RegisterSpellScript(spell_ascension_reaper_redshade_reap);
}
