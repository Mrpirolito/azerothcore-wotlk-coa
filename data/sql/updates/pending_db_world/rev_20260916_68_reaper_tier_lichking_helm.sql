-- The Reaper tier helm takes the Lich King's crown.
--
-- The Crown of Domination has no item model in 3.3.5 - it lives on the Lich King creature - so the
-- art comes from the "Lich King Armor 3.X.X" patch, which ships only models, textures and icons and
-- no DBC of its own: Item\OBJECTCOMPONENTS\Head\Helm_Plate_LichKing_A_01_<race><gender>.m2 for all
-- twenty race and gender pairs, Helm_Plate_LichKing_A_01Normal.blp, and Interface\ICONS\
-- INV_LichKing_Helmet.blp. It needs a display record to be reachable, and it had none.
--
-- 990000 is cloned from 56636, the Sanctified Ymirjar Lord's Helmet, so the helmet geoset visibility
-- that hides hair and facial hair under a raid plate helm carries over unchanged; only the model,
-- the texture and the icon differ.
--
-- The client needs the same row in its own ItemDisplayInfo.dbc before it will draw any of this,
-- which is the matching client patch, and it needs patch-Z.mpq installed for the art itself.
DELETE FROM `itemdisplayinfo_dbc` WHERE `ID` = 990000;
INSERT INTO `itemdisplayinfo_dbc`
  (`ID`, `ModelName_1`, `ModelTexture_1`, `InventoryIcon_1`, `GeosetGroup_1`, `Flags`,
   `SpellVisualID`, `GroupSoundIndex`, `HelmetGeosetVis_1`, `HelmetGeosetVis_2`)
VALUES
  (990000, 'Helm_Plate_LichKing_A_01.mdx', 'Helm_Plate_LichKing_A_01Normal', 'INV_LichKing_Helmet',
   0, 0, 0, 11, 248, 306);

UPDATE `item_template` SET `displayid` = 990000
WHERE `entry` IN (992001, 992011, 992021, 992031);
