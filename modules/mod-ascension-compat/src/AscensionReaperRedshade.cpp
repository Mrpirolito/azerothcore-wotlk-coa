/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
//
// The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has transformed
// into Thresh!". Casting Thresh then applies Bloodshatter (Dummy) 525299 for the second step. Both
// are dummies as far as the server is concerned, so all three parts of the promise are ours: the
// cast has to be legal, the button has to say what the cast will do, and neither can announce
// itself in chat.
//
// The cast: CMSG_CAST_SPELL is refused outright for a spell the caster does not have active -
// "cheater? kick? ban?" - so a button that merely looks like Thresh spends a global cooldown and
// does nothing. A Reaper never learns Thresh or Bloodshatter, because they exist only as what
// Redshade turns Reap into, so owning the talent is what puts them in the spellbook, once, for as
// long as the talent is taken. Player::addSpell with learnFromSkill set is the way in that sends
// no packet: Player::learnSpell answers a temporary learn with SMSG_LEARNED_SPELL, which the
// client prints as "You have learned a new ability: Thresh".
//
// The button: SMSG_ACTION_BUTTONS, the packet a druid's forms already use to swap a whole bar.
// The bar the client is given is the character's own, with the ranks of Reap reading as the
// replacement; putting it back is the character's own bar again, unchanged. The obvious
// alternative, SMSG_SUPERCEDED_SPELL - which is what Player::SetTemporarySpellReplacement sends -
// prints that same learned-ability line for whichever spell it names, including the rank of Reap
// it hands the button back to, so it announced the transform twice per cast.
//
// And spell_ascension_reaper_redshade_reap covers the other direction: a client that still sends
// the rank on the bar gets the replacement cast for it.

#include "Player.h"
#include "Opcodes.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <algorithm>
#include <array>

namespace
{
constexpr uint32 SPELL_THRESH_DUMMY = 525058;
constexpr uint32 SPELL_THRESH = 505170;
constexpr uint32 SPELL_BLOODSHATTER_DUMMY = 525299;
constexpr uint32 SPELL_BLOODSHATTER = 505326;

constexpr std::array<uint32, 9> ReapRanks = { 354319, 500357, 504056, 504057, 504058, 504557,
    505151, 573302, 573303 };

bool IsReap(uint32 spellId)
{
    return std::find(ReapRanks.begin(), ReapRanks.end(), spellId) != ReapRanks.end();
}

// The character's own bar, with every button that holds a rank of Reap reading as the replacement.
void SendTransformedBar(Player* player, uint32 replacement)
{
    if (!player->GetSession())
        return;

    WorldPacket data(SMSG_ACTION_BUTTONS, 1 + (MAX_ACTION_BUTTONS * 4));
    data << uint8(1); // button data follows, as the core sends after a spec swap
    for (uint8 button = 0; button < MAX_ACTION_BUTTONS; ++button)
    {
        ActionButton const* action = player->GetActionButton(button);
        if (!action)
        {
            data << uint32(0);
            continue;
        }

        uint32 packed = action->packedData;
        if (action->GetType() == ACTION_BUTTON_SPELL && IsReap(action->GetAction()))
            packed = replacement | (uint32(ACTION_BUTTON_SPELL) << 24);

        data << uint32(packed);
    }

    player->GetSession()->SendPacket(&data);
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
            SendTransformedBar(player, Replacement());
    }

    void Remove(AuraEffect const* /*effect*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player)
            return;

        // The second step replaces the same buttons and is applied before this one is removed, so
        // the bar only goes back once no step is left to own it.
        uint32 const other = GetId() == SPELL_BLOODSHATTER_DUMMY ? SPELL_THRESH_DUMMY
            : SPELL_BLOODSHATTER_DUMMY;
        if (player->HasAura(other))
            return;

        player->SendActionButtons(1);
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
