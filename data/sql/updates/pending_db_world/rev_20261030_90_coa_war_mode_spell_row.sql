-- 1004119 is the War Mode aura. The apex Stormfang "Frenzy" row used that id and replaced War Mode's
-- SpellInfo, so selecting War Mode failed. Frenzy moves to the free id 1004120.
DELETE FROM `spell_dbc` WHERE `ID` = 1004120;
UPDATE `spell_dbc` SET `ID` = 1004120 WHERE `ID` = 1004119;
