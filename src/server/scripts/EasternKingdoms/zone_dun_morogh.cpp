/*
* Copyright (C) 2005-2011 MaNGOS <http://www.getmangos.com/>
*
* Copyright (C) 2008-2011 Trinity <http://www.trinitycore.org/>
*
* Copyright (C) 2006-2011 ScriptDev2 <http://www.scriptdev2.com/>
*
* Copyright (C) 2010-2011 Project SkyFire <http://www.projectskyfire.org/>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

/* ScriptData
SDName: Dun_Morogh
SD%Complete: 50
SDComment: Quest support: 1783
SDCategory: Dun Morogh
EndScriptData */

/* ContentData
npc_narm_faulk
npc_survivor
npc_carvo_blastbolt
npc_safe_operative
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "Vehicle.h"
#include "Player.h"
#include "GameObject.h"
#include "Creature.h"

/*######
## npc_narm_faulk
######*/

// signed for 6172
enum eNarmFaulkData
{
	SAY_HEAL = -1000187,
};

class npc_narm_faulk : public CreatureScript
{
public:
	npc_narm_faulk() : CreatureScript("npc_narm_faulk") { }

	CreatureAI* GetAI(Creature* pCreature) const
	{
		return new npc_narm_faulkAI(pCreature);
	}

	struct npc_narm_faulkAI : public ScriptedAI
	{
		uint32 lifeTimer;
		bool spellHit;

		npc_narm_faulkAI(Creature *c) : ScriptedAI(c) {}

		void Reset()
		{
			lifeTimer = 120000;
			me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
			me->SetStandState(UNIT_STAND_STATE_DEAD);
			spellHit = false;
		}

		void EnterCombat(Unit * /*who*/)
		{
		}

		void MoveInLineOfSight(Unit * /*who*/)
		{
			return;
		}

		void UpdateAI(uint32 diff) override
		{
			if (me->IsStandState())
			{
				if (lifeTimer <= diff)
				{
					EnterEvadeMode();
					return;
				}
				else
					lifeTimer -= diff;
			}
		}

		void SpellHit(Unit * Hitter, SpellInfo const* Spellkind) override
		{
			if (Spellkind->Id == 8593 && !spellHit)
			{
				DoCast(me, 32343);
				me->SetStandState(UNIT_STAND_STATE_STAND);
				me->SetUInt32Value(UNIT_DYNAMIC_FLAGS, 0);
				//me->RemoveAllAuras();
				//DoScriptText(SAY_HEAL, me);
				spellHit = true;
			}
		}
	};
};

/*######
## npc_gs_9x_multi_bot
######*/

enum eMultiBotData
{
	OBJECT_TOXIC_POOL = 203975,
	SPELL_CLEAN_UP_TOXIC_POOL = 79424,
	SPELL_TOXIC_POOL_CREDIT_TO_MASTER = 79422,
	SAY_MULTI_BOT = -1610001
};

class npc_gs_9x_multi_bot : public CreatureScript
{
public:
	npc_gs_9x_multi_bot() : CreatureScript("npc_gs_9x_multi_bot") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_gs_9x_multi_botAI(creature);
	}

	struct npc_gs_9x_multi_botAI : public ScriptedAI
	{
		npc_gs_9x_multi_botAI(Creature* c) : ScriptedAI(c) { }

		void UpdateAI(uint32 diff) override
		{
			GameObject* pool = me->FindNearestGameObject(OBJECT_TOXIC_POOL, 2.0f);

			if (pool)
			{
				if (Player* player = me->GetCharmerOrOwnerPlayerOrPlayerItself())
				{
					me->MonsterSay(SAY_MULTI_BOT, LANG_UNIVERSAL, 0);
					me->CastSpell(me, SPELL_CLEAN_UP_TOXIC_POOL, true);
					me->CastSpell(player, SPELL_TOXIC_POOL_CREDIT_TO_MASTER, true);
					pool->Delete();
				}
			}
		}
	};
};

/*######
## npc_survivor
######*/

class npc_survivor : public CreatureScript
{
public:
	npc_survivor() : CreatureScript("npc_survivor") { }

	CreatureAI* GetAI(Creature* creature) const
	{
		return new npc_survivorAI(creature);
	}

	struct npc_survivorAI : public ScriptedAI
	{
		npc_survivorAI(Creature* c) : ScriptedAI(c) { }

		void SpellHit(Unit * caster, SpellInfo const* spell) override
		{
			if (spell->Id == 86264)
				me->DespawnOrUnsummon(1000);
		}
	};
};

/*######
## npc_carvo_blastbolt
######*/

class npc_carvo_blastbolt : public CreatureScript
{
public:
	npc_carvo_blastbolt() : CreatureScript("npc_carvo_blastbolt") { }

	bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
	{
		//sLog->outMessage("carvo_blastbolt onQuestAccept is triggered.", LOG_LEVEL_DEBUG, 0);
		if (quest->GetQuestId() == 28169)
			player->SummonCreature(47836, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), 0, TEMPSUMMON_MANUAL_DESPAWN);

		return true;
	}
};

/*######
## npc_safe_operative
######*/

class npc_safe_operative : public CreatureScript
{
public:

	npc_safe_operative() : CreatureScript("npc_safe_operative") { }

	CreatureAI* GetAI(Creature* c) const
	{
		return new npc_safe_operativeAI(c);
	}

	struct npc_safe_operativeAI : public npc_escortAI
	{
		npc_safe_operativeAI(Creature* c) : npc_escortAI(c)
		{
			Start(false, true);
		}

		void WaypointReached(uint32 uiPointId)
		{
			if (uiPointId == 5)
			{
				me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
				me->DespawnOrUnsummon(10000);
			}
		}
	};

};

/*######
## npc_sanitron_500
######*/
/*

TODO:
* Make npcs to cast spells when sanitron reaches waypoints.
* Make player to not be able to control sanitron.

*/

float sanitron_points[4][3] =
{
	{ -5173.00f, 731.92f, 292.58f },
	{ -5174.05f, 721.53f, 292.58f },
	{ -5174.60f, 712.68f, 292.58f },
	{ -5175.51f, 703.60f, 292.58f }
};

class npc_sanitron_500 : public CreatureScript
{
public:
	npc_sanitron_500() : CreatureScript("npc_sanitron_500") { }

	VehicleAI* GetAI(Creature* c) const
	{
		return new npc_sanitron_500AI(c);
	}

	struct npc_sanitron_500AI : public VehicleAI
	{
		npc_sanitron_500AI(Creature* pCreature) : VehicleAI(pCreature)
		{
			hasPassenger = false;
			continueWP = true;
			me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
			currentPoint = 0;
		}

		bool continueWP, wait, hasPassenger;
		uint32 currentPoint, waitTimer;

		void Reset()
		{
			hasPassenger = false;
			continueWP = true;
			wait = false;
			currentPoint = 0;
			//me->RemoveVehicleKit();
			//me->GetVehicleKit()->Die();
			me->GetVehicleKit()->Install();
		}

		void MovementInform(uint32 type, uint32 id)
		{

			if (type != POINT_MOTION_TYPE)
				return;

			switch (id)
			{
			case 0:
				wait = true;
				waitTimer = 3000;
				++currentPoint;
				break;
			case 1:
			case 2:
				wait = true;
				waitTimer = 5000;
				++currentPoint;
				break;
			case 3:
				wait = true;
				waitTimer = 3000;
				++currentPoint;
				if (hasPassenger)
				{
					me->GetVehicleKit()->GetPassenger(0)->RemoveAura(80653);
					me->GetVehicleKit()->GetPassenger(0)->ToPlayer()->AreaExploredOrEventHappens(27635);
				}
				Reset();
				me->Kill(me, false);
				break;
			default:
				break;
			}
		}

		void UpdateAI(uint32 diff) override
		{

			if (!me->GetVehicleKit()->HasEmptySeat(0))
				hasPassenger = true;
			else
				hasPassenger = false;

			if (hasPassenger)
			{
				if (continueWP)
				{
					me->GetMotionMaster()->MovePoint(currentPoint, sanitron_points[currentPoint][0], sanitron_points[currentPoint][1], sanitron_points[currentPoint][2]);
					continueWP = false;
				}

				if (wait)
				{
					if (waitTimer <= diff)
					{
						continueWP = true;
						wait = false;
					}
					else waitTimer -= diff;
				}
			}
		}
	};
};

enum MissingInAction
{
	QUEST_MISSING_IN_ACTION = 26284,
	NPC_DEMOLITIONIST_PRISONER = 42645,
};

class go_demolitionist_cage : public GameObjectScript
{
public:
	go_demolitionist_cage() : GameObjectScript("go_demolitionist_cage") { }

	bool OnGossipHello(Player* player, GameObject* go) override
	{
		
		go->SetGoState(GO_STATE_READY);

		if (Creature* prisoner = go->FindNearestCreature(NPC_DEMOLITIONIST_PRISONER, 1.0f, true))
		{
			go->UseDoorOrButton();
			
			if (player->GetQuestStatus(QUEST_MISSING_IN_ACTION) == QUEST_STATUS_INCOMPLETE)
			{
				if (player)
					player->KilledMonsterCredit(NPC_DEMOLITIONIST_PRISONER, NULL);

				prisoner->GetMotionMaster()->MovePoint(1, go->GetPositionX(), go->GetPositionY() - 8, go->GetPositionZ());
				prisoner->DespawnOrUnsummon(4000);
				go->ResetDoorOrButton();
				return false;
				
			}
		}
		return true;
	}
};

void AddSC_dun_morogh()
{
	new npc_narm_faulk();
	new npc_gs_9x_multi_bot();
	new npc_survivor();
	new npc_carvo_blastbolt();
	new npc_safe_operative();
	new npc_sanitron_500();
	new go_demolitionist_cage();
}