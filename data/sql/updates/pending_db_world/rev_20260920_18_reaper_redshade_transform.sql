-- Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
--
-- The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads that Reap has
-- transformed into Thresh. That aura is a plain SPELL_AURA_DUMMY, so nothing ever acted on
-- it: the buff appeared and the button still said Reap. Casting Thresh then applies
-- Bloodshatter (Dummy) 525299 for the second step, with the same problem.
--
-- aura_ascension_reaper_redshade_transform answers both through
-- Player::SetTemporarySpellReplacement, so the button is redrawn and the cast handler sends
-- the replacement. It binds to the two buffs rather than to the ranks of Reap.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
  ('spell_ascension_reaper_redshade_reap', 'aura_ascension_reaper_redshade_transform');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(525058, 'aura_ascension_reaper_redshade_transform'),
(525299, 'aura_ascension_reaper_redshade_transform');
