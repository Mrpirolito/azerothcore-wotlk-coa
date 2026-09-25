-- Blood Shards (804849, Bloodmage level 30 passive): "Shadow Damage dealt now generates 1 Blood Shard, plus 1 additional
-- if it critically strikes, up to 8. Dealing damage with Veinburst will now expend all stacks of Blood Shards, each
-- dealing ${$504115m1+$AP*0.05+$SP*.1}". Nothing implemented the shards: the passive's proc aura has no proc flags
-- and its generator (506640) is a dummy. bloodmage_blood_shards now keeps the count on 505366 (8 stacks) with one
-- orbiting visual per shard (505349-505356); the shards end when the first visual's duration runs out.
-- Talents built on them: Battleweaver 801963, Inhumane 807488, Everlasting Hunt 804686.
DELETE FROM `spell_script_names` WHERE `spell_id` = 505349 AND `ScriptName` = 'aura_ascension_bloodmage_blood_shard_expiry';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES (505349, 'aura_ascension_bloodmage_blood_shard_expiry');
