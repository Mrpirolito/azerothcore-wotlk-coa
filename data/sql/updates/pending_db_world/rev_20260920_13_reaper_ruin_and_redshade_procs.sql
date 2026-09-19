-- Ruin and Redshade never triggered, for the same reason Extinction did not.
--
-- Both are SPELL_AURA_PROC_TRIGGER_SPELL passives whose DBC record carries ProcTypeMask 0, so
-- SpellMgr builds the aura with no proc flags and nothing can reach the handler. The talents are
-- owned, their tooltips read correctly, and neither has ever fired.
--
--   805198 Ruin      triggers 805199, the debuff that raises Doomrend and Slaughter damage taken
--   524735 Redshade  triggers 525058, the Thresh transform
--
-- spell_proc supplies the flags the records omit rather than editing the client's Spell.dbc.
-- Chance 100 in both cases: the scripts decide which spell qualifies, because a proc flag alone
-- cannot say "Shudder Scythe" or "Reap".

-- Ruin reads "Damage dealt by Shudder Scythe", so it wants a landed damaging hit:
-- ProcFlags 16 = PROC_FLAG_DONE_SPELL_MELEE_DMG_CLASS, SpellTypeMask 1, SpellPhaseMask 2.
DELETE FROM `spell_proc` WHERE `SpellId` = 805198;
INSERT INTO `spell_proc`
  (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
   `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
   `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
  (805198, 0, 0, 0, 0, 0, 16, 1, 2, 0, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 805198
  AND `ScriptName` = 'spell_ascension_reaper_ruin';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(805198, 'spell_ascension_reaper_ruin');

-- Redshade reads "Using Reap", which is the cast and not the hit: SpellPhaseMask 1.
DELETE FROM `spell_proc` WHERE `SpellId` = 524735;
INSERT INTO `spell_proc`
  (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
   `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
   `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
  (524735, 0, 0, 0, 0, 0, 16, 1, 1, 0, 0, 0, 0, 100, 0, 0);

DELETE FROM `spell_script_names` WHERE `spell_id` = 524735
  AND `ScriptName` = 'spell_ascension_reaper_redshade';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(524735, 'spell_ascension_reaper_redshade');
