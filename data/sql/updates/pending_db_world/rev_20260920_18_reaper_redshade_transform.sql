-- Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
--
-- The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads that Reap has
-- transformed into Thresh. That aura is a plain SPELL_AURA_DUMMY, so nothing ever acted on
-- it: the buff appeared and the button still said Reap. Casting Thresh then applies
-- Bloodshatter (Dummy) 525299 for the second step, with the same problem.
--
-- aura_ascension_reaper_redshade_transform answers both through
-- Player::SetTemporarySpellReplacement, so the button is redrawn and the cast handler sends
-- the replacement. aura_ascension_reaper_redshade_spells puts the two transformed abilities
-- in the spellbook for as long as the talent is owned: teaching them per transform instead
-- announced "You have learned" and "You have unlearned" in chat on every single Reap.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
  ('spell_ascension_reaper_redshade_reap', 'aura_ascension_reaper_redshade_transform',
   'aura_ascension_reaper_redshade_spells');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(524735, 'aura_ascension_reaper_redshade_spells'),
(525058, 'aura_ascension_reaper_redshade_transform'),
(525299, 'aura_ascension_reaper_redshade_transform');
