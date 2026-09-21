/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// Reaper talents whose proc never fired.
//
// Each of these is a SPELL_AURA_PROC_TRIGGER_SPELL passive whose DBC record carries
// ProcTypeMask 0. SpellMgr then builds the aura with no proc flags, so the handler is
// installed and no event can ever reach it: the talent is learnt, its tooltip reads
// correctly, and it does nothing. Only four of the Reaper's proc talents ship with flags
// of their own, so this is the rule rather than the exception.
//
// spell_proc supplies the flags, rather than editing the Spell.dbc the realm shares with
// every player. A proc flag can say "a melee ability landed"; it cannot say which ability
// it was, and the talents have no family mask to select with. So each row hands every
// qualifying event to one script, and this table says which spells the talent answers to.
// An empty list means any spell the flags already allow, and a chance other than 100 -
// Soulsight and both ranks of Soulforged Weaponry - belongs in the row, not here.
//
// A cast-phase row needs the DONE_SPELL_*_DMG_CLASS_POS flags as well as the _NEG pair,
// because casting a beneficial spell on yourself - Spectre Stride, Harvest Time, Bolstered
// Form - raises only the positive ones. The spell list still decides which cast counts.

#ifndef ASCENSION_REAPER_TALENT_PROCS_H
#define ASCENSION_REAPER_TALENT_PROCS_H

#include <array>
#include <cstdint>

namespace AscensionReaperTalentProcs
{
struct Rule
{
    std::uint32_t Talent;
    std::array<std::uint32_t, 26> Spells;   // zero terminated; all zero means "anything"
};

// The spell ids are every rank of the ability the tooltip names.
inline constexpr std::array<Rule, 30> Rules =
{{
    // Dealing direct damage
    {300565, {{0}}},
    // Dealing damage with Reliquary of the Lost
    {301193, {{500629, 500630, 500631, 500765}}},
    // Avoiding attacks
    {500286, {{0}}},
    // Casting Spectral Scythe
    {504309, {{500484, 500576, 561293}}},
    // Critical strikes with Dirge and Murder
    {520056, {{573029, 573037, 801311, 801328, 802796, 803834, 803835, 803836, 803837, 803838, 803839, 500376, 502679, 502680, 502681, 502682, 502683, 502684, 504622, 560421, 573030, 804270}}},
    // Damage dealt with Soul Strike
    {524939, {{500517, 500518, 500519, 500520, 500521, 500522, 500574, 500646}}},
    // Casting Spectre Stride
    {560412, {{551544, 705458, 707659, 801624, 801835, 802422, 802423, 802424, 802425, 802426, 802427, 802428, 803742, 803942}}},
    // Casting Bolstered Form
    {560478, {{680337, 705434}}},
    // Your successful parries and dodges
    {560487, {{0}}},
    // Damage dealt with Soul Strike
    {560919, {{500517, 500518, 500519, 500520, 500521, 500522, 500574, 500646}}},
    // Your auto attacks
    {561127, {{0}}},
    // Casting Withering Touch
    {561170, {{573071}}},
    // Your auto attacks
    {561340, {{0}}},
    // Casting Wraithblade
    {680996, {{805258, 806818, 806819, 806820, 806821, 806822, 806823, 806824, 807166}}},
    // Every 6th cast of Reap
    {704193, {{354319, 500357, 504056, 504057, 504058, 504557, 505151, 573302, 573303, 801327}}},
    // Damage dealt with Dreadwake
    {704356, {{503286, 503287, 503288, 503289, 503324, 503531, 803992}}},
    // Casting Spectre Stride, Scythe Rush or Veilwalk
    {704357, {{551544, 705458, 707659, 801624, 801835, 802422, 802423, 802424, 802425, 802426, 802427, 802428, 803742, 803942, 500359, 500372, 500377, 500538, 805339, 805689, 803990, 803991}}},
    // Critical strikes with Doomrend
    {704552, {{502668, 502669, 502670, 502671, 560361, 567531, 567532, 800172, 800173}}},
    // Direct critical strikes with Shadow damage
    {704558, {{0}}},
    // Requiem and Soulrend
    {705403, {{572179, 572341, 572342, 573316, 573317, 573318, 573319, 573320, 573321, 573322, 802731, 805717, 806840, 806841, 806842, 806843, 806844, 806845}}},
    // Your Slaughter now has a chance to cast an additional time
    {705414, {{500373, 500429, 500430, 500431, 500432, 500433, 500434}}},
    // Casting Sinister Litany
    {705426, {{805185}}},
    // Critical strikes with Slaughter
    {705428, {{500373, 500429, 500430, 500431, 500432, 500433, 500434}}},
    // Casting Wraithblade
    {705437, {{805258, 806818, 806819, 806820, 806821, 806822, 806823, 806824, 807166}}},
    // Damage dealt by Soul Strike and Murder
    {706792, {{500376, 500517, 500518, 500519, 500520, 500521, 500522, 500574, 500646, 502679, 502680, 502681, 502682, 502683, 502684, 504622, 560421, 573030, 804270}}},
    // Parrying an attack
    {706795, {{0}}},
    // Direct damage critical strikes
    {707707, {{0}}},
    // Striking at least 1 enemy with Soulslam
    {712484, {{504014, 504685}}},
    // Damage dealt with Reap, Dreadwake and Soul Strike
    {801325, {{354319, 500357, 504056, 504057, 504058, 504557, 505151, 573302, 573303, 801327, 503286, 503287, 503288, 503289, 503324, 503531, 803992, 500517, 500518, 500519, 500520, 500521, 500522, 500574, 500646}}},
    // Damage dealt by Deathchaser
    {805196, {{560351, 560428, 560429, 561032, 561033, 561034, 573052, 573053, 805190, 807546}}},
}};
}

#endif // ASCENSION_REAPER_TALENT_PROCS_H
