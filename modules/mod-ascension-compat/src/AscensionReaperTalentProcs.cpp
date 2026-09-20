/* Copyright (C) 2016+ AzerothCore, GNU AGPL v3. */

// One script for every Reaper talent in AscensionReaperTalentProcs.h.
//
// Their spell_proc rows carry the flags the DBC records omit and a chance of 100, so every
// qualifying event arrives here and this decides whether it was the right spell. A proc flag
// can say "a melee ability landed"; it cannot say which ability, and these talents have no
// family mask to select with.
//
// A rule with an empty spell list accepts whatever its flags and hit mask already allow -
// that is how "avoiding attacks" and "your successful parries and dodges" work, where the
// event is the dodge itself rather than a named ability.

#include "AscensionReaperTalentProcs.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include <algorithm>

namespace
{
using AscensionReaperTalentProcs::Rules;

class spell_ascension_reaper_talent_proc : public AuraScript
{
    PrepareAuraScript(spell_ascension_reaper_talent_proc);

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        auto rule = std::find_if(Rules.begin(), Rules.end(),
            [this](AscensionReaperTalentProcs::Rule const& entry)
            {
                return entry.Talent == GetId();
            });
        if (rule == Rules.end())
            return false;

        if (!rule->Spells[0])
            return true;

        SpellInfo const* spellInfo = eventInfo.GetSpellInfo();
        if (!spellInfo)
            return false;

        return std::find(rule->Spells.begin(), rule->Spells.end(), spellInfo->Id) !=
            rule->Spells.end();
    }

    void Register() override
    {
        DoCheckProc += AuraCheckProcFn(spell_ascension_reaper_talent_proc::CheckProc);
    }
};
}

void AddSC_AscensionReaperTalentProcs()
{
    RegisterSpellScript(spell_ascension_reaper_talent_proc);
}
