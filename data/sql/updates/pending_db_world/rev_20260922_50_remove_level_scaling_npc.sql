-- Remove the Keeper of Proportion.
--
-- rev_20260916_63 added creature 990010 as an in-world switch for creature and quest level
-- scaling, standing at the Valley of Strength entrance. The realm does not want it: the two
-- settings it flips are already in mod_ascension_compat.conf, and the NPC is one more body at
-- a doorway people walk through.
--
-- The two training dummies placed beside it in rev_20260916_69 stay. They are the ones the
-- realm tests abilities on, and they have nothing to do with the switch.

DELETE FROM `creature` WHERE `guid` = 5400003;
DELETE FROM `creature_template_model` WHERE `CreatureID` = 990010;
DELETE FROM `creature_template` WHERE `entry` = 990010;
DELETE FROM `npc_text` WHERE `ID` = 990010;
