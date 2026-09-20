-- Haunt (573425) summoned the Witch Doctor's Mirage, creature 840000, whose script opens with
--
--     if (!player || player->getClass() != CLASS_WITCH_DOCTOR)
--         return;
--
-- so a Reaper got none of it: no copied appearance, no faction, no owner, no movement. The visage
-- stood where it was cast as a neutral level one creature.
--
-- 990020 is the Reaper's own visage, driven by npc_ascension_reaper_haunt, so neither class has to
-- know about the other. Level, faction and appearance are taken from the caster when it is
-- summoned; the values here only cover the moment before that runs.
DELETE FROM `creature_template_model` WHERE `CreatureID` = 990020;
DELETE FROM `creature_template` WHERE `entry` = 990020;
INSERT INTO `creature_template`
  (`entry`, `name`, `subname`, `gossip_menu_id`, `minlevel`, `maxlevel`, `faction`, `npcflag`,
   `speed_walk`, `speed_run`, `unit_class`, `unit_flags`, `type`, `AIName`, `MovementType`,
   `flags_extra`, `ScriptName`)
VALUES
  (990020, 'Haunt', NULL, 0, 80, 80, 35, 0, 1, 1.14286, 1, 0, 7, '', 0, 0,
   'npc_ascension_reaper_haunt');

INSERT INTO `creature_template_model`
  (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`)
VALUES
  (990020, 0, 11686, 1, 1);
