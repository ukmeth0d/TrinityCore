/*
* Copyright (C) 2012-2013 HTCore <http://cata.vfire-core.com/>
* Copyright (C) 2012-2013 WoW Source <http://wow.amgi-it.ro/>
* by Shee Shen
*/

#include "ScriptMgr.h"

enum Texts
{
    SAY_AGGRO = 0,
    SAY_DEATH = 1,
    SAY_EVENT = 2,
};

enum CreatureIds
{
    MOB_HORROR = 40787,
	MOB_SCARAB = 39440,
};

enum Spells
{
    SPELL_FLAME_BOLT = 77370,
    SPELL_RAGING_SMASH = 83650,
	SPELL_EARTH_POINT = 75339,
	SPELL_DUST_MOVE = 75546, // Effect Dust Move + damage
	SPELL_VORTEX_DUST = 78515, // Effect Vortex + damage
};

enum Phases
{
    PHASE_ALL = 0,
    PHASE_NORMAL = 1,
    PHASE_WATERSPOUT = 2,
};

class boss_ptah : public CreatureScript
{
public:
    boss_ptah() : CreatureScript("boss_ptah") { }

    struct boss_ptahAI : public ScriptedAI
    {
        boss_ptahAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
            pInstance = pCreature->GetInstanceScript();
        }

        std::list<uint64> SummonList;

        InstanceScript *pInstance;

        uint8 Phase;
        bool Phased;
        uint8 SpawnCount;
        uint8 PhaseCount;

        //uint32 FlameBoltTimer;
        uint32 SmashTimer;
        uint32 SummonPointTimer;
        uint32 Phase2EndTimer;

        void Reset()
        {
            Phased = false;
            RemoveSummons();

            Phase = PHASE_NORMAL;

            SpawnCount = 6;
            PhaseCount = 0;

            //FlameBoltTimer = urand(15000,27000);
            SmashTimer = urand(4000, 10000);
            SummonPointTimer = urand(11000,16000);

            me->RemoveAurasDueToSpell(SPELL_VORTEX_DUST);
            me->RemoveAurasDueToSpell(SPELL_DUST_MOVE);

            me->GetMotionMaster()->MoveTargetedHome();
        }

        void SummonedCreatureDespawn(Creature* summon)
        {
            switch(summon->GetEntry())
            {
                case MOB_SCARAB:
                case MOB_HORROR:
                    SpawnCount--;
                    break;
            }
        }

        void RemoveSummons()
        {
            if (SummonList.empty())
                return;

            for (std::list<uint64>::const_iterator itr = SummonList.begin(); itr != SummonList.end(); ++itr)
            {
                if (Creature* pTemp = ObjectAccessor::GetCreature(*me, *itr))
                    if (pTemp)
                        pTemp->DisappearAndDie();
            }
            SummonList.clear();
        }

        void JustSummoned(Creature* pSummon)
        {
            switch (pSummon->GetEntry())
            {
                case MOB_SCARAB:
                case MOB_HORROR:
                    if (Unit* pTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        pSummon->AI()->AttackStart(pTarget);
                    SummonList.push_back(pSummon->GetGUID());
                break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            Talk(SAY_AGGRO);
        }

        void JustDied(Unit* /*pKiller*/)
        {
            Talk(SAY_DEATH);
			RemoveSummons();
        }

        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (SpawnCount == 0 && Phase == PHASE_WATERSPOUT)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, false);
                SpawnCount = 6;
                SetCombatMovement(true);
                Phase = PHASE_NORMAL;
                Phased = false;
                //FlameBoltTimer = urand(15000,27000);
                SmashTimer = urand(4000, 10000);
                SummonPointTimer = urand(15000,21000);
                me->RemoveAurasDueToSpell(SPELL_VORTEX_DUST);
                me->RemoveAurasDueToSpell(SPELL_DUST_MOVE);
				me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            }

            if (me->HealthBelowPct(50) && Phase == PHASE_NORMAL && PhaseCount == 0)
            {
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
                PhaseCount++;
                SetCombatMovement(false);
                Phase = PHASE_WATERSPOUT;
				me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                me->AddAura(SPELL_VORTEX_DUST, me);
                me->AddAura(SPELL_DUST_MOVE, me);
                Position pos;
                me->GetPosition();
                me->SummonCreature(MOB_SCARAB, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(MOB_SCARAB, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
				me->SummonCreature(MOB_SCARAB, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(MOB_SCARAB, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                me->SummonCreature(MOB_HORROR, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
				me->SummonCreature(MOB_HORROR, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
                Phase2EndTimer = 60000;
            }

            /*if (FlameBoltTimer <= diff && Phase == PHASE_NORMAL)
            {
                DoCastAOE(SPELL_FLAME_BOLT);
                FlameBoltTimer = urand(15000,27000);
            } else FlameBoltTimer -= diff;*/

            if (SmashTimer <= diff && Phase == PHASE_NORMAL)
            {
                DoCast(me->GetVictim(), SPELL_RAGING_SMASH);
                SmashTimer = urand(4000, 10000);
            } else SmashTimer -= diff;

            if (SummonPointTimer <= diff && Phase == PHASE_NORMAL)
            {
                Talk(SAY_EVENT);
				if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, -10.0f, true))
				DoCast(target, SPELL_EARTH_POINT);
                SummonPointTimer = urand(15000,21000);
            } else SummonPointTimer -= diff;

            if (Phase == PHASE_WATERSPOUT)
            {
                if (Phase2EndTimer <= diff)
                {
                    SpawnCount = 3;
                    SetCombatMovement(true);
                    Phase = PHASE_NORMAL;
                    Phased = false;
                    //FlameBoltTimer = urand(15000,27000);
                    SmashTimer = urand(4000, 10000);
                    SummonPointTimer = urand(15000,21000);
                    me->RemoveAurasDueToSpell(SPELL_VORTEX_DUST);
                    me->RemoveAurasDueToSpell(SPELL_DUST_MOVE);
                } else Phase2EndTimer -= diff;
            }

            if(!PHASE_WATERSPOUT)
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature *pCreature) const
    {
        return new boss_ptahAI (pCreature);
    }
};

void AddSC_boss_ptah()
{
    new boss_ptah();
}