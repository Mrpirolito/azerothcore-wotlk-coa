-- Redshade turns Reap into Thresh, and Thresh into Bloodshatter.
--
-- The talent's proc applies Thresh (Dummy) 525058, whose tooltip reads "Your Reap has
-- transformed into Thresh!". That aura is a plain SPELL_AURA_DUMMY, so nothing ever acted
-- on it: the buff appeared and Reap stayed Reap. Casting Thresh then applies Bloodshatter
-- (Dummy) 525299 for the second step, with the same problem.
--
-- spell_ascension_reaper_redshade_reap answers both, on every rank of Reap.
DELETE FROM `spell_script_names` WHERE `ScriptName` = 'spell_ascension_reaper_redshade_reap';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(354319, 'spell_ascension_reaper_redshade_reap'),
(500357, 'spell_ascension_reaper_redshade_reap'),
(504056, 'spell_ascension_reaper_redshade_reap'),
(504057, 'spell_ascension_reaper_redshade_reap'),
(504058, 'spell_ascension_reaper_redshade_reap'),
(504557, 'spell_ascension_reaper_redshade_reap'),
(505151, 'spell_ascension_reaper_redshade_reap'),
(573302, 'spell_ascension_reaper_redshade_reap'),
(573303, 'spell_ascension_reaper_redshade_reap'),
(801327, 'spell_ascension_reaper_redshade_reap');
