-- Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
--
-- The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads that Reap has
-- transformed into Thresh. That aura is a plain SPELL_AURA_DUMMY, so nothing ever acted on
-- it: the buff appeared and the button still said Reap. Casting Thresh then applies
-- Bloodshatter (Dummy) 525299 for the second step, with the same problem.
--
-- aura_ascension_reaper_redshade_transform redraws the button on each step, and
-- spell_ascension_reaper_redshade_reap sends the replacement from every rank of Reap, so the
-- right ability goes out whichever spell the client asks for. Not 801327: it is named Reap
-- as well, and its rank reads "Heal" - a self-targeted heal with nothing to do with the
-- strike, so replacing it would have eaten a heal the player asked for.
DELETE FROM `spell_script_names` WHERE `ScriptName` IN
  ('spell_ascension_reaper_redshade_reap', 'aura_ascension_reaper_redshade_transform',
   'aura_ascension_reaper_redshade_spells');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(525058, 'aura_ascension_reaper_redshade_transform'),
(525299, 'aura_ascension_reaper_redshade_transform'),
(354319, 'spell_ascension_reaper_redshade_reap'),
(500357, 'spell_ascension_reaper_redshade_reap'),
(504056, 'spell_ascension_reaper_redshade_reap'),
(504057, 'spell_ascension_reaper_redshade_reap'),
(504058, 'spell_ascension_reaper_redshade_reap'),
(504557, 'spell_ascension_reaper_redshade_reap'),
(505151, 'spell_ascension_reaper_redshade_reap'),
(573302, 'spell_ascension_reaper_redshade_reap'),
(573303, 'spell_ascension_reaper_redshade_reap');
