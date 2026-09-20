-- Soulstone Lure (561376) summoned nothing at all.
--
-- Its only effect is SPELL_EFFECT_SUMMON of creature 557911, and that creature has no
-- creature_template row. Player::SummonCreature finds no template, so the cast goes through, the
-- cooldown starts and no lure ever appears.
--
-- The template follows the ones the other custom classes already use for a planted object the
-- player leaves behind - Spirit Link Idol (522106) and Cauldron Hidden Periodic (506011): faction
-- 35 so nothing attacks it by faction alone, type 11 (not specified), no movement, and a level
-- band that keeps it out of level scaling.
DELETE FROM `creature_template_model` WHERE `CreatureID` = 557911;
DELETE FROM `creature_template` WHERE `entry` = 557911;
INSERT INTO `creature_template`
  (`entry`, `name`, `subname`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
   `speed_walk`, `speed_run`, `unit_class`, `unit_flags`, `type`, `AIName`, `MovementType`,
   `flags_extra`, `ScriptName`)
VALUES
  (557911, 'Soulstone Lure', NULL, 0, 80, 80, 35, 0, 1, 1.14286, 1, 0, 11, '', 0, 0, '');

-- 410144 is the Spirit Link Idol's model: a small planted object that reads as something left on
-- the ground rather than a creature standing on it.
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`) VALUES
(557911, 0, 410144, 1, 1);

-- "Enemies who attack the lure are horrified" is the second half, and it never fired either.
-- 561826 is the aura the lure carries: effect 1 is the periodic taunt 562313, effect 2 is a
-- SPELL_AURA_PROC_TRIGGER_SPELL for the Fear 561827, and its record has ProcTypeMask 0, so the
-- aura is built with no proc flags and nothing reaches the handler.
--
-- ProcFlags 1048848 = PROC_FLAG_TAKEN_MELEE_AUTO_ATTACK (0x100000) |
-- PROC_FLAG_TAKEN_SPELL_MELEE_DMG_CLASS (0x200) | PROC_FLAG_TAKEN_SPELL_NONE_DMG_CLASS_NEG
-- (0x40) - the three ways an enemy can attack the lure.
DELETE FROM `spell_proc` WHERE `SpellId` = 561826;
INSERT INTO `spell_proc`
  (`SpellId`, `SchoolMask`, `SpellFamilyName`, `SpellFamilyMask0`, `SpellFamilyMask1`,
   `SpellFamilyMask2`, `ProcFlags`, `SpellTypeMask`, `SpellPhaseMask`, `HitMask`, `AttributesMask`,
   `DisableEffectsMask`, `ProcsPerMinute`, `Chance`, `Cooldown`, `Charges`)
VALUES
  (561826, 0, 0, 0, 0, 0, 1048848, 3, 2, 0, 0, 0, 0, 100, 0, 0);

-- 410144 was the Spirit Link Idol's model, which is not what this is. 408534 is
-- creature\demoncrystal\creature_demoncrystal_03_blue, a blue crystal, which is what the ability
-- places.
UPDATE `creature_template_model` SET `CreatureDisplayID` = 408534 WHERE `CreatureID` = 557911;
