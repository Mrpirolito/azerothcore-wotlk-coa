-- Three local placements and the tier set's vendor.

-- 1. The training dummies move from the outer gate to the Valley of Strength entrance, where the
--    character actually stands: (1481.73, -4395.51, 26.68), read from characters.position_* rather
--    than guessed. The second sits three yards along the same contour.
DELETE FROM `creature` WHERE `guid` IN (5400001, 5400002);
INSERT INTO `creature`
  (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `equipment_id`,
   `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`,
   `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `Comment`)
VALUES
  (5400001, 31144, 1, 0, 0, 1, 1, 0, 1481.73, -4395.51, 26.68, 6.229, 300, 0, 0, 1, 0, 0, 'CoA local - Orgrimmar entrance training dummy (level 80)'),
  (5400002, 31146, 1, 0, 0, 1, 1, 0, 1484.70, -4396.50, 26.60, 6.229, 300, 0, 0, 1, 0, 0, 'CoA local - Orgrimmar entrance training dummy (level 83 boss)');

-- 2. The level scaling NPC moves out of the bank to the same entrance, a few yards to the side so it
--    does not stand on the dummies. (1481.00, -4405.00, 25.60) sits between the character's own
--    position and the Orgrimmar Grunt already spawned at (1479.39, -4406.25, 25.56), so the ground
--    height comes from a confirmed spawn.
UPDATE `creature`
SET `position_x` = 1481.00, `position_y` = -4405.00, `position_z` = 25.60, `orientation` = 3.100
WHERE `guid` = 5400003;

-- 3. Horace Hunderland (35498) sells the Reaper tier.
--
-- Prices reuse extended costs that already exist in the client, so the vendor shows a real price
-- without a client patch: 2739/2741 are 30 and 50 Emblems of Frost, 2740/2742 are 60 and 95. Head,
-- chest and legs cost the larger amount and shoulders and hands the smaller, the way Icecrown's own
-- tier is priced. 277 and 284 both sit at 95 for now - gating them behind a Mark of Sanctification
-- needs two new ItemExtendedCost records in the client as well, which is a separate patch.
DELETE FROM `npc_vendor` WHERE `entry` = 35498 AND `item` BETWEEN 992001 AND 992099;
INSERT INTO `npc_vendor` (`entry`, `slot`, `item`, `maxcount`, `incrtime`, `ExtendedCost`) VALUES
-- item level 251
(35498, 0, 992001, 0, 0, 2741), (35498, 0, 992002, 0, 0, 2739), (35498, 0, 992003, 0, 0, 2741),
(35498, 0, 992004, 0, 0, 2741), (35498, 0, 992005, 0, 0, 2739),
-- item level 264
(35498, 0, 992011, 0, 0, 2742), (35498, 0, 992012, 0, 0, 2740), (35498, 0, 992013, 0, 0, 2742),
(35498, 0, 992014, 0, 0, 2742), (35498, 0, 992015, 0, 0, 2740),
-- item level 277
(35498, 0, 992021, 0, 0, 2742), (35498, 0, 992022, 0, 0, 2740), (35498, 0, 992023, 0, 0, 2742),
(35498, 0, 992024, 0, 0, 2742), (35498, 0, 992025, 0, 0, 2740),
-- item level 284
(35498, 0, 992031, 0, 0, 2742), (35498, 0, 992032, 0, 0, 2740), (35498, 0, 992033, 0, 0, 2742),
(35498, 0, 992034, 0, 0, 2742), (35498, 0, 992035, 0, 0, 2740);
