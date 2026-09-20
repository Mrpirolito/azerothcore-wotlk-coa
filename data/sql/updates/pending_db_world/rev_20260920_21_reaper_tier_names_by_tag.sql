-- The green source line is what says which raid a piece came from, so the name does not
-- repeat it. The 284 pieces already dropped their prefix; these two kept theirs, and a piece
-- read "Mythic Frostbitten Helm of the Risen Nightmare" above a green "Mythic".
UPDATE `item_template` SET `name` = TRIM(LEADING 'Mythic ' FROM `name`)
WHERE `entry` BETWEEN 992021 AND 992025 AND `name` LIKE 'Mythic %';

UPDATE `item_template` SET `name` = TRIM(LEADING 'Sanctified ' FROM `name`)
WHERE `entry` BETWEEN 992011 AND 992015 AND `name` LIKE 'Sanctified %';

UPDATE `item_set_names` SET `name` = TRIM(LEADING 'Mythic ' FROM `name`)
WHERE `entry` BETWEEN 992021 AND 992025 AND `name` LIKE 'Mythic %';

UPDATE `item_set_names` SET `name` = TRIM(LEADING 'Sanctified ' FROM `name`)
WHERE `entry` BETWEEN 992011 AND 992015 AND `name` LIKE 'Sanctified %';

-- The realm's own tag for that difficulty is "Mythic Raid", which 1966 items carry. A bare
-- "Mythic" is what 13446 items of every other kind use, so the 277 pieces read as if they
-- had come from anywhere.
UPDATE `item_template` SET `description` = '@Mythic Raid@'
WHERE `entry` BETWEEN 992021 AND 992025 AND `description` = '@Mythic@';
