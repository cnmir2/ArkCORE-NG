/*
* Copyright (C) 2011-2016 ArkCORE <http://www.arkania.net/>
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#include "ScriptPCH.h"
#include "Cell.h"
#include "CellImpl.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObjectAI.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "PassiveAI.h"
#include "Pet.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptedEscortAI.h"
#include "ScriptedFollowerAI.h"
#include "Unit.h"
#include "Vehicle.h"

// 34864
class npc_gilneas_city_guard_gate_34864 : public CreatureScript
{
public:
    npc_gilneas_city_guard_gate_34864() : CreatureScript("npc_gilneas_city_guard_gate_34864") { }

    enum eNpc
    {
        NPC_PANICKED_CITIZEN_GATE = 44086,
        EVENT_START_TALK_WITH_CITIZEN = 101,
        EVENT_TALK_WITH_CITIZEN_1,
        EVENT_TALK_WITH_CITIZEN_2,
        EVENT_TALK_WITH_CITIZEN_3,
    };

    struct npc_gilneas_city_guard_gate_34864AI : public ScriptedAI
    {
        npc_gilneas_city_guard_gate_34864AI(Creature* creature) : ScriptedAI(creature)  { }

        EventMap m_events;
        uint8    m_say;
        uint8    m_emote;
        uint64	 m_citicenGUID;

        void Reset() override
        {
            if (me->GetDistance2d(-1430.47f, 1345.55f) < 10.0f)
                m_events.ScheduleEvent(EVENT_START_TALK_WITH_CITIZEN, urand(10000, 30000));
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_TALK_WITH_CITIZEN:
                    {
                        m_citicenGUID = GetRandomCitizen();
                        m_emote = RAND(EMOTE_STATE_COWER, EMOTE_STATE_TALK, EMOTE_ONESHOT_CRY, EMOTE_STATE_SPELL_PRECAST, EMOTE_STATE_EXCLAIM);
                        m_say = urand(0, 2);
                        if (Creature* npc = sObjectAccessor->GetCreature(*me, m_citicenGUID))
                            npc->HandleEmoteCommand(m_emote);
                        m_events.ScheduleEvent(EVENT_TALK_WITH_CITIZEN_1, urand(1200, 2000));
                        break;
                    }
                    case EVENT_TALK_WITH_CITIZEN_1:
                    {
                        if (Creature* npc = sObjectAccessor->GetCreature(*me, m_citicenGUID))
                            npc->AI()->Talk(m_say);
                        m_events.ScheduleEvent(EVENT_TALK_WITH_CITIZEN_2, 5000);
                        break;
                    }
                    case EVENT_TALK_WITH_CITIZEN_2:
                    {
                        Talk(m_say);
                        m_events.ScheduleEvent(EVENT_TALK_WITH_CITIZEN_3, 5000);
                        break;
                    }
                    case EVENT_TALK_WITH_CITIZEN_3:
                    {
                        if (Creature* npc = sObjectAccessor->GetCreature(*me, m_citicenGUID))
                            npc->HandleEmoteCommand(EMOTE_STATE_NONE);
                        m_events.ScheduleEvent(EVENT_START_TALK_WITH_CITIZEN, urand(5000, 30000));
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void FillCitizenList()
        {
            listOfCitizenGUID.clear();
            std::list<Creature*> listOfCitizen;
            me->GetCreatureListWithEntryInGrid(listOfCitizen, NPC_PANICKED_CITIZEN_GATE, 35.0f);
            for (std::list<Creature*>::iterator itr = listOfCitizen.begin(); itr != listOfCitizen.end(); ++itr)
                listOfCitizenGUID.push_back((*itr)->GetGUID());
        }

        uint64 GetRandomCitizen()
        {
            if (listOfCitizenGUID.empty())
                FillCitizenList();
            uint8 rol = urand(0, listOfCitizenGUID.size() - 1);
            std::list<uint64>::iterator itr = listOfCitizenGUID.begin();
            std::advance(itr, rol);
            return (*itr);
        }

    private:
        std::list<uint64> listOfCitizenGUID;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_gate_34864AI(creature);
    }
};

// 34850
class npc_prince_liam_greymane_34850 : public CreatureScript
{
public:
    npc_prince_liam_greymane_34850() : CreatureScript("npc_prince_liam_greymane_34850") { }

    enum eNpc
    {
        QUEST_LOCKDOWN = 14078,
        NPC_PANICKED_CITIZEN = 34851,
        EVENT_START_TALK_TO_GUARD = 101,
        EVENT_TALK_TO_GUARD_1,
        EVENT_TALK_TO_GUARD_2,
        EVENT_TALK_TO_GUARD_3,
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_LOCKDOWN)
            if (Creature* citizen = creature->FindNearestCreature(NPC_PANICKED_CITIZEN, 20.0f))
                citizen->AI()->Talk(0);

        return true;
    }

    struct npc_prince_liam_greymane_34850AI : public ScriptedAI
    {
        npc_prince_liam_greymane_34850AI(Creature *c) : ScriptedAI(c) {}

        EventMap m_events;

        void Reset() override
        {
            m_events.ScheduleEvent(EVENT_START_TALK_TO_GUARD, 60000);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_TALK_TO_GUARD:
                    {
                        Talk(0);
                        m_events.ScheduleEvent(EVENT_TALK_TO_GUARD_1, 15000);
                        break;
                    }
                    case EVENT_TALK_TO_GUARD_1:
                    {
                        Talk(1);
                        m_events.ScheduleEvent(EVENT_TALK_TO_GUARD_2, 18000);
                        break;
                    }
                    case EVENT_TALK_TO_GUARD_2:
                    {
                        Talk(2);
                        m_events.ScheduleEvent(EVENT_TALK_TO_GUARD_3, 25000);
                        break;
                    }
                    case EVENT_TALK_TO_GUARD_3:
                    {
                        Talk(0);
                        m_events.ScheduleEvent(EVENT_START_TALK_TO_GUARD, urand(45000, 60000));
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_34850AI(creature);
    }
};

/* 35660 // part showfight worgen <> liam */   /* Quest 14098 */
class npc_rampaging_worgen_35660 : public CreatureScript
{
public:
    npc_rampaging_worgen_35660() : CreatureScript("npc_rampaging_worgen_35660") { }

    enum eNpc
    {
        NPC_PRINCE_LIAM = 34913,
        NPC_FRIGHTENED_CITIZEN_WORGEN = 35836,
        GO_MERCHANT_DOOR = 195327,
        PLAYER_GUID = 99999,
        SPELL_ENRAGE = 56646,

        MOVE_TO_START_POSITION = 101,
        MOVE_TO_PRINCE_LIAM,  
        MOVE_TO_DOOR,

        EVENT_MOVE_TO_LIAM = 101,
        EVENT_ATTACK_LIAM,
        EVENT_ENRAGE_COOLDOWN,
        EVENT_MOVE_TO_DOOR,
        EVENT_FOLLOW_CITIZEN1,
        EVENT_FOLLOW_CITIZEN2,

        ACTION_START_ANIM_MERCANT = 101,
        ACTION_START_ANIM_LIAM = 102,
    };

    struct npc_rampaging_worgen_35660AI : public ScriptedAI
    {
        npc_rampaging_worgen_35660AI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_events;
        bool m_enrage;
        uint64 m_liamGUID;
        uint64 m_citizenGUID;
        uint64 m_doorGUID;

        void Reset() override
        {           
            m_enrage = false;
            m_doorGUID = NULL;
            m_liamGUID = NULL;
            m_citizenGUID = NULL;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void JustDied(Unit* /*killer*/) override
        {
            me->DespawnOrUnsummon(1000);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type == POINT_MOTION_TYPE)
                switch (id)
            {
                case MOVE_TO_START_POSITION:
                {
                    m_events.ScheduleEvent(EVENT_MOVE_TO_LIAM, 500);
                    break;
                }
                case MOVE_TO_PRINCE_LIAM:
                {
                    m_events.ScheduleEvent(EVENT_ATTACK_LIAM, 100);
                    break;
                }
                case MOVE_TO_DOOR:
                {
                    m_events.ScheduleEvent(EVENT_FOLLOW_CITIZEN1, 3000);
                    break;
                }
            }
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (!m_enrage && me->GetHealthPct() < 90.0f)
            {
                me->CastSpell(me, SPELL_ENRAGE);
                m_enrage = true;
                m_events.ScheduleEvent(EVENT_ENRAGE_COOLDOWN, urand(121000, 150000));
            }
        }

        void AttackStart(Unit* who) override
        {
            if (who->GetEntry() != NPC_PRINCE_LIAM)
                return;
            ScriptedAI::AttackStart(who);
        }

        void SetGUID(uint64 guid, int32 id = 0) override
        { 
            switch (id)
            {
                case NPC_PRINCE_LIAM:
                {
                    m_liamGUID = guid;
                    break;
                }
                case NPC_FRIGHTENED_CITIZEN_WORGEN:
                {
                    m_citizenGUID = guid;
                    break;
                }
                case GO_MERCHANT_DOOR:
                {
                    m_doorGUID = guid;
                    break;
                }
            }
        }

        void DoAction(int32 param) override
        {
            switch (param)
            {
                case ACTION_START_ANIM_MERCANT:
                {
                    m_events.ScheduleEvent(EVENT_MOVE_TO_DOOR, 1000);
                    break;
                }
                case ACTION_START_ANIM_LIAM:
                {
                    me->SetSpeed(MOVE_RUN, 1.8f, true);
                    me->GetMotionMaster()->MovePoint(MOVE_TO_START_POSITION, -1482.9f, 1394.6f, 35.55f);
                    break;
                }              
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENRAGE_COOLDOWN:
                    {
                        m_enrage = false;
                        break;
                    }
                    case EVENT_MOVE_TO_LIAM:
                    {
                        if (Creature* liam = sObjectAccessor->GetCreature(*me, m_liamGUID))
                        {
                            Position pos = liam->GetPosition();
                            float angle = liam->GetAngle(pos.GetPositionX(), pos.GetPositionY());
                            Position pos2 = liam->GetNearPosition(2.0f, angle);
                            me->GetMotionMaster()->MovePoint(MOVE_TO_PRINCE_LIAM, pos2, true);
                        }
                        break;
                    }
                    case EVENT_ATTACK_LIAM:
                    {
                        if (Creature* liam = sObjectAccessor->GetCreature(*me, m_liamGUID))
                        {
                            me->SetReactState(REACT_AGGRESSIVE);
                            me->Attack(liam, true);
                            liam->Attack(me, true);
                        }
                        break;
                    }
                    case EVENT_MOVE_TO_DOOR:
                    {
                        if (GameObject* go = sObjectAccessor->GetGameObject(*me, m_doorGUID))
                            me->GetMotionMaster()->MovePoint(MOVE_TO_DOOR, go->GetPosition());
                        break;
                    }
                    case EVENT_FOLLOW_CITIZEN1:
                    {
                        if (Creature* citizen = sObjectAccessor->GetCreature(*me, m_citizenGUID))
                            me->GetMotionMaster()->MoveFollow(citizen, 1.0f, 0.0f);
                        m_events.ScheduleEvent(EVENT_FOLLOW_CITIZEN2, 5000);
                        break;
                    }
                    case EVENT_FOLLOW_CITIZEN2:
                    {
                        me->DespawnOrUnsummon();
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen_35660AI(creature);
    }
};

// 34913 // part showfight liam <> worgen
class npc_prince_liam_greymane_34913 : public CreatureScript
{
public:
    npc_prince_liam_greymane_34913() : CreatureScript("npc_prince_liam_greymane_34913") { }

    enum eNpc
    {
        NPC_RAMPAGING_WORGEN = 35660,
        EVENT_START_NEXT_SHOWFIGHT = 101,
        ACTION_START_ATTACK_LIAM = 102,
    };

    struct npc_prince_liam_greymane_34913AI : public ScriptedAI
    {
        npc_prince_liam_greymane_34913AI(Creature* c) : ScriptedAI(c) { Initialize(); }

        EventMap m_events;
        uint64 m_worgenGUID;

        void Initialize()
        {
            m_events.ScheduleEvent(EVENT_START_NEXT_SHOWFIGHT, 60000);
            m_worgenGUID = NULL;
        }

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            damage /= 3;
        }

        void JustSummoned(Creature* summon) override
        { 
            if (summon->GetEntry() == NPC_RAMPAGING_WORGEN)
            {
                m_worgenGUID = summon->GetGUID();
                summon->GetAI()->SetGUID(me->GetGUID(), me->GetEntry());   
                summon->GetAI()->DoAction(ACTION_START_ATTACK_LIAM);
            }
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        { 
            m_worgenGUID = NULL;
            if (summon->GetEntry() == NPC_RAMPAGING_WORGEN)
                m_events.ScheduleEvent(EVENT_START_NEXT_SHOWFIGHT, 20000);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_NEXT_SHOWFIGHT:
                    {                        
                        me->SummonCreature(NPC_RAMPAGING_WORGEN, -1491.9f, 1413.1f, 35.56f, 5.5f, TEMPSUMMON_TIMED_DESPAWN, 120000);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_prince_liam_greymane_34913AI(creature);
    }
};

// 34916 // part showfight guard <> worgen..
class npc_gilneas_city_guard_34916 : public CreatureScript
{
public:
    npc_gilneas_city_guard_34916() : CreatureScript("npc_gilneas_city_guard_34916") { }

    enum eNpc
    {
        NPC_RAMPAGING_WORGEN_34884 = 34884,
        NPC_BLOODFANG_WORGEN_35118 = 35118,
        NPC_RAMPAGING_WORGEN_35660 = 35660,
        EVENT_CHECK_SHOWFIGHT = 101,
        MOVE_TO_HOMEPOSITION = 101,
    };

    struct npc_gilneas_city_guard_34916AI : public ScriptedAI
    {
        npc_gilneas_city_guard_34916AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        EventMap m_events;
        float    m_minHealthPct;
        std::list<uint32> m_worgenList;

        void Initialize()
        {
            m_worgenList.clear();
            m_worgenList.push_back(NPC_RAMPAGING_WORGEN_34884);
            m_worgenList.push_back(NPC_BLOODFANG_WORGEN_35118);
            m_worgenList.push_back(NPC_RAMPAGING_WORGEN_35660);
            m_minHealthPct = frand(30.0f, 85.0f);
        }

        void Reset() override
        {
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, urand(900, 1200));
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void DamageTaken(Unit* who, uint32 &Damage) override
        {
            if (who->ToCreature())
                if (me->GetHealthPct() < m_minHealthPct)
                    Damage = 0;
                else
                    Damage = 1;
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType damageType) override
        {
            if (victim->ToCreature())
                if (victim->GetHealthPct() < m_minHealthPct)
                    damage = 0;
                else
                    damage = 1;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE && pointId == MOVE_TO_HOMEPOSITION)
                m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 25);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_SHOWFIGHT:
                    {
                        if (me->IsDead() || me->IsInCombat())
                        {
                            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                            return;
                        }

                        if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 10.0f)
                        {
                            me->GetMotionMaster()->MovePoint(MOVE_TO_HOMEPOSITION, me->GetHomePosition());
                            return;
                        }

                        if (Creature* worgen = me->FindNearestCreature(m_worgenList, 5.0f))
                            if (!worgen->IsInCombat())
                            {
                                me->Attack(worgen, true);
                                worgen->Attack(me, true);
                                m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                                return;
                            }

                        m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilneas_city_guard_34916AI(creature);
    }
};

// 34884 // part showfight worgen <> guard..
class npc_rampaging_worgen_34884 : public CreatureScript
{
public:
    npc_rampaging_worgen_34884() : CreatureScript("npc_rampaging_worgen_34884") {}

    enum eNpc
    {
        EVENT_ENRAGE_COOLDOWN,
        SPELL_ENRAGE = 8599,
    };

    struct npc_rampaging_worgen_34884AI : public ScriptedAI
    {
        npc_rampaging_worgen_34884AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        bool m_enrage;

        void Reset() override
        {
            m_enrage = false;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (!m_enrage && me->GetHealthPct() < 90.0f)
            {
                me->CastSpell(me, SPELL_ENRAGE);
                m_enrage = true;
                m_events.ScheduleEvent(EVENT_ENRAGE_COOLDOWN, urand(121000, 150000));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENRAGE_COOLDOWN:
                    {
                        m_enrage = false;
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_rampaging_worgen_34884AI(creature);
    }
};

// 34981 // alone
class npc_frightened_citizen_34981 : public CreatureScript
{
public:
    npc_frightened_citizen_34981() : CreatureScript("npc_frightened_citizen_34981") {} 

    enum eNpc
    {
        NPC_FRIGHTENED_CITIZEN_34981 = 34981,
        NPC_GILNEAS_EVACUATION_FACING_MARKER_35830 = 35830,
        NPC_FRIGHTENED_CITIZEN_WORGEN = 35836,
        NPC_RAMPAGING_WORGEN = 35660,
        GO_MERCHANT_DOOR = 195327,
        PLAYER_GUID = 99999,

        ACTION_START_ANIM_CITIZEN = 103,

        EVENT_MOVE_TO_DOOR = 101,
        EVENT_MOVE_TO_PLAYER,
        EVENT_MOVE_TO_MARKER,
        EVENT_MOVE_TO_END,

        MOVE_TO_DOOR = 101,
        MOVE_TO_PLAYER,
        MOVE_TO_MARKER,
        MOVE_TO_END,
    };

    struct npc_frightened_citizen_34981AI : public ScriptedAI
    {
        npc_frightened_citizen_34981AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        uint64 m_playerGUID;
        uint64 m_doorGUID;

        void Reset() override
        {
            m_playerGUID = NULL;
            m_doorGUID = NULL;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void MovementInform(uint32 type, uint32 point) override
        {
            if (type == POINT_MOTION_TYPE)
                switch (point)
                {
                    case MOVE_TO_DOOR:
                    {
                        m_events.ScheduleEvent(EVENT_MOVE_TO_PLAYER, 25);
                        break;
                    }
                    case MOVE_TO_PLAYER:
                    {
                        Talk(1);
                        m_events.ScheduleEvent(EVENT_MOVE_TO_MARKER, 4000);
                        break;
                    }
                    case MOVE_TO_MARKER:
                    {
                        m_events.ScheduleEvent(EVENT_MOVE_TO_END, 25);
                        break;
                    }
                    case MOVE_TO_END:
                    {
                        me->DespawnOrUnsummon();
                        break;
                    }
                }
        }

        void SetGUID(uint64 guid, int32 id = 0) override
        {
            switch (id)
            {
                case PLAYER_GUID:
                {
                    m_playerGUID = guid;
                    break;
                }
                case GO_MERCHANT_DOOR:
                {
                    m_doorGUID = guid;
                    break;
                }
            }
        }

        void DoAction(int32 param) override
        {
            switch (param)
            {
                case ACTION_START_ANIM_CITIZEN:
                {
                    m_events.ScheduleEvent(EVENT_MOVE_TO_DOOR, 2000);
                    break;
                }
            }
        }
        
        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_TO_DOOR:
                    {
                        if (GameObject* go = sObjectAccessor->GetGameObject(*me, m_doorGUID))
                            me->GetMotionMaster()->MovePoint(MOVE_TO_DOOR, go->GetPosition());
                        break;
                    }
                    case EVENT_MOVE_TO_PLAYER:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            me->GetMotionMaster()->MovePoint(MOVE_TO_PLAYER, player->GetNearPosition(2.0, player->GetOrientation()));
                        break;
                    }
                    case EVENT_MOVE_TO_MARKER:
                    {
                        if (Creature* marker = me->FindNearestCreature(NPC_GILNEAS_EVACUATION_FACING_MARKER_35830, 100.0f))
                            me->GetMotionMaster()->MovePoint(MOVE_TO_MARKER, marker->GetPosition());
                        break;
                    }
                    case EVENT_MOVE_TO_END:
                    {
                        me->GetMotionMaster()->MovePoint(MOVE_TO_END, me->GetNearPosition(25.0f, 0.0f));
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frightened_citizen_34981AI(creature);
    }
};

// 35836 // with worgen
class npc_frightened_citizen_35836 : public CreatureScript
{
public:
    npc_frightened_citizen_35836() : CreatureScript("npc_frightened_citizen_35836") {} 

    enum eNpc
    {
        NPC_FRIGHTENED_CITIZEN_34981 = 34981,
        NPC_GILNEAS_EVACUATION_FACING_MARKER_35830 = 35830,
        NPC_FRIGHTENED_CITIZEN_WORGEN = 35836,
        NPC_RAMPAGING_WORGEN = 35660,
        GO_MERCHANT_DOOR = 195327,
        PLAYER_GUID = 99999,

        ACTION_START_ANIM_CITIZEN = 103,

        EVENT_MOVE_TO_DOOR = 101,
        EVENT_MOVE_TO_PLAYER,
        EVENT_MOVE_TO_MARKER,
        EVENT_MOVE_TO_END,

        MOVE_TO_DOOR = 101,
        MOVE_TO_PLAYER,
        MOVE_TO_MARKER,
        MOVE_TO_END,
    };

    struct npc_frightened_citizen_35836AI : public ScriptedAI
    {
        npc_frightened_citizen_35836AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        uint64 m_playerGUID;
        uint64 m_doorGUID;

        void Reset() override
        {
            m_playerGUID = NULL;
            m_doorGUID = NULL;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        }

        void MovementInform(uint32 type, uint32 point) override
        {
            if (type == POINT_MOTION_TYPE)
                switch (point)
                {
                    case MOVE_TO_DOOR:
                    {
                        m_events.ScheduleEvent(EVENT_MOVE_TO_PLAYER, 25);
                        break;
                    }
                    case MOVE_TO_PLAYER:
                    {
                        Talk(0);
                        m_events.ScheduleEvent(EVENT_MOVE_TO_MARKER, 2000);
                        break;
                    }
                    case MOVE_TO_MARKER:
                    {
                        m_events.ScheduleEvent(EVENT_MOVE_TO_END, 25);
                        break;
                    }
                    case MOVE_TO_END:
                    {
                        me->DespawnOrUnsummon();
                        break;
                    }
                }
        }

        void SetGUID(uint64 guid, int32 id = 0) override
        {
            switch (id)
            {
                case PLAYER_GUID:
                {
                    m_playerGUID = guid;
                    break;
                }
                case GO_MERCHANT_DOOR:
                {
                    m_doorGUID = guid;
                    break;
                }
            }
        }

        void DoAction(int32 param) override
        {
            switch (param)
            {
                case ACTION_START_ANIM_CITIZEN:
                {
                    m_events.ScheduleEvent(EVENT_MOVE_TO_DOOR, 1000);
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_TO_DOOR:
                    {
                        if (GameObject* go = sObjectAccessor->GetGameObject(*me, m_doorGUID))
                            me->GetMotionMaster()->MovePoint(MOVE_TO_DOOR, go->GetPosition());
                        break;
                    }
                    case EVENT_MOVE_TO_PLAYER:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                        {
                            me->GetMotionMaster()->MovePoint(MOVE_TO_PLAYER, player->GetNearPosition(2.0, player->GetOrientation()));
                        }
                        break;
                    }
                    case EVENT_MOVE_TO_MARKER:
                    {
                        if (Creature* marker = me->FindNearestCreature(NPC_GILNEAS_EVACUATION_FACING_MARKER_35830, 100.0f))
                            me->GetMotionMaster()->MovePoint(MOVE_TO_MARKER, marker->GetPosition());
                        break;
                    }
                    case EVENT_MOVE_TO_END:
                    {
                        me->GetMotionMaster()->MovePoint(MOVE_TO_END, me->GetNearPosition(10.0f, 0.0f));
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frightened_citizen_35836AI(creature);
    }
};

// 195327
class go_merchant_square_door_195327 : public GameObjectScript
{
public:
    go_merchant_square_door_195327() : GameObjectScript("go_merchant_square_door_195327") { }

    enum eDoor
    {
        NPC_FRIGHTENED_CITIZEN_WORGEN = 35836,
        NPC_FRIGHTENED_CITIZEN_ALONE = 34981,
        NPC_RAMPAGING_WORGEN = 35660,
        PLAYER_GUID = 99999,
        GO_MERCHANT_DOOR = 195327,
        ACTION_START_ANIM_MERCANT = 101,
        ACTION_START_ANIM_CITIZEN = 103,
    };

    uint64 m_playerGUID;
    uint64 m_doorGUID;
    uint64 m_citizenGUID;
    uint64 m_worgenGUID;

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        if (!player || !go)
            return false;

        switch (urand(0, 1))
        {
            case 0:
            {
                SummonOnlyCitizen(player, go);
                break;
            }
            case 1:
            {
                SummonCitizenAndWorgen(player, go);
                break;
            }
        }

        return false;
    }

private:
    void SummonOnlyCitizen(Player* player, GameObject* go)
    {
        Position posC = GetCitizenPosition(go);
        if (Creature* citizen = player->SummonCreature(NPC_FRIGHTENED_CITIZEN_ALONE, posC, TEMPSUMMON_TIMED_DESPAWN, 30000))
        {
            citizen->GetAI()->SetGUID(go->GetGUID(), go->GetEntry());
            citizen->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
            citizen->GetAI()->DoAction(ACTION_START_ANIM_CITIZEN);
        }       
    }

    void SummonCitizenAndWorgen(Player* player, GameObject* go)
    {
        Position posC = GetCitizenPosition(go);
        Position posW = GetWorgenPosition(go);
        if (Creature* citizen = player->SummonCreature(NPC_FRIGHTENED_CITIZEN_WORGEN, posC, TEMPSUMMON_TIMED_DESPAWN, 30000))
            if (Creature* worgen = player->SummonCreature(NPC_RAMPAGING_WORGEN, posW, TEMPSUMMON_TIMED_DESPAWN, 30000))
            {
                citizen->GetAI()->SetGUID(go->GetGUID(), go->GetEntry());
                citizen->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
                citizen->GetAI()->SetGUID(worgen->GetGUID(), worgen->GetEntry());
                citizen->GetAI()->DoAction(ACTION_START_ANIM_CITIZEN);
                worgen->GetAI()->SetGUID(go->GetGUID(), go->GetEntry());
                worgen->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
                worgen->GetAI()->SetGUID(citizen->GetGUID(), citizen->GetEntry());
                worgen->GetAI()->DoAction(ACTION_START_ANIM_MERCANT);
            }
    }

    Position GetCitizenPosition(GameObject* go)
    {
        float x, y;
        go->GetNearPoint2D(x, y, 1.0f, go->GetOrientation() + M_PI);
        return Position(x, y, go->GetPositionZ());
    }

    Position GetWorgenPosition(GameObject* go)
    {
        float x, y;
        go->GetNearPoint2D(x, y, 3.0f, go->GetOrientation() + M_PI * 0.75f);
        return Position(x, y, go->GetPositionZ());
    }
};

// 35872
class npc_mariam_spellwalker_35872 : public CreatureScript
{
public:
    npc_mariam_spellwalker_35872() : CreatureScript("npc_mariam_spellwalker_35872") {}

    enum eNpc
    {
        NPC_BLOODFANG_WORGEN = 35118,
        SPELL_FROSTBOLT_VISUAL_ONLY = 74277,
        EVENT_CHECK_SPELL = 101,
        AI_MIN_HP = 85,
    };

    struct npc_mariam_spellwalker_35872AI : public ScriptedAI
    {
        npc_mariam_spellwalker_35872AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;

        void Reset() override
        {
            m_events.ScheduleEvent(EVENT_CHECK_SPELL, urand(1000, 2000));
        }

        void DamageTaken(Unit* who, uint32& damage) override
        {
            if (me->HealthBelowPct(AI_MIN_HP) && who->ToCreature())
                damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_SPELL:
                    {
                        if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                            if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 5.0f, true))
                                me->AI()->AttackStart(enemy); // She should really only grab agro when npc Cleese is not there, so we will keep this range small

                        m_events.ScheduleEvent(EVENT_CHECK_SPELL, urand(1000, 2000));
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            if (me->GetVictim()->GetEntry() == NPC_BLOODFANG_WORGEN)
                DoSpellAttackIfReady(SPELL_FROSTBOLT_VISUAL_ONLY); //Dummy spell, visual only to prevent getting agro (Blizz-like)
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_mariam_spellwalker_35872AI(creature);
    }
};

// 35232
class npc_gilnean_royal_guard_35232 : public CreatureScript
{
public:
    npc_gilnean_royal_guard_35232() : CreatureScript("npc_gilnean_royal_guard_35232") {}

    enum eNpc
    {
        NPC_RAMPAGING_WORGEN_34884 = 34884,
        NPC_BLOODFANG_WORGEN_35118 = 35118,
        NPC_RAMPAGING_WORGEN_35660 = 35660,
        NPC_KING_GREYMANE_35112 = 35112,

        EVENT_CHECK_SHOWFIGHT = 101,
        EVENT_CHECK_KING_NEAR,

        MOVE_TO_HOMEPOSITION = 101,
    };

    struct npc_gilnean_royal_guard_35232AI : public ScriptedAI
    {
        npc_gilnean_royal_guard_35232AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        EventMap m_events;
        float    m_minHealthPct;
        bool     m_isShowFight;
        bool     m_theKingIsNear;
        uint64   m_kingGUID;
        std::list<uint32> m_worgenList;

        void Initialize()
        {
            m_worgenList.clear();
            m_worgenList.push_back(NPC_RAMPAGING_WORGEN_34884);
            m_worgenList.push_back(NPC_BLOODFANG_WORGEN_35118);
            m_worgenList.push_back(NPC_RAMPAGING_WORGEN_35660);
            m_theKingIsNear = false;
            m_isShowFight = true;
            m_kingGUID = NULL;
            m_minHealthPct = frand(30.0f, 85.0f);
        }

        void Reset() override
        {
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 1000);
            m_events.ScheduleEvent(EVENT_CHECK_KING_NEAR, 1000);
        }

        void DamageTaken(Unit* who, uint32 &Damage) override
        {
            if (!m_theKingIsNear)
                if (m_isShowFight && who->ToCreature())
                    if (me->GetHealthPct() < m_minHealthPct)
                        Damage = 0;
                    else
                        Damage = 1;
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType damageType) override
        { 
            if (!m_theKingIsNear)
                if (m_isShowFight && victim->ToCreature())
                    if (victim->GetHealthPct() < m_minHealthPct)
                        damage = 0;
                    else
                        damage = 1;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE && pointId == MOVE_TO_HOMEPOSITION)
                m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 25);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_SHOWFIGHT:
                    {
                        if (me->IsDead() || me->IsInCombat())
                        {
                            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                            return;
                        }

                        if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 10.0f)
                        {
                            me->GetMotionMaster()->MovePoint(MOVE_TO_HOMEPOSITION, me->GetHomePosition());
                            return;
                        }
                       
                        if (Creature* worgen = me->FindNearestCreature(m_worgenList, 5.0f))
                        {
                            me->Attack(worgen, true);
                            worgen->Attack(me, true);
                            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                            return;
                        }
                        
                        m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                        break;
                    }
                    case EVENT_CHECK_KING_NEAR:
                    {
                        if (Creature* king = me->FindNearestCreature(NPC_KING_GREYMANE_35112, 15.0f))
                        {
                            m_kingGUID = king->GetGUID();
                            m_theKingIsNear = true;
                            m_isShowFight = false;
                        }
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }


    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_royal_guard_35232AI(creature);
    }
};

// 35839
class npc_sergeant_cleese_35839 : public CreatureScript
{
public:
    npc_sergeant_cleese_35839() : CreatureScript("npc_sergeant_cleese_35839") {}

    enum eNpc
    {
        NPC_BLOODFANG_WORGEN = 35118,
        AI_MIN_HP = 85,
    };

    struct npc_sergeant_cleese_35839AI : public ScriptedAI
    {
        npc_sergeant_cleese_35839AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tSeek;

        void Reset() override
        {
            tSeek = urand(1000, 2000);
        }

        void DamageTaken(Unit* who, uint32& damage) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                me->getThreatManager().resetAllAggro();
                who->AddThreat(me, 1.0f);
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (who->IsPet())
            {
                me->getThreatManager().resetAllAggro();
                me->AddThreat(who, 1.0f);
                me->AI()->AttackStart(who);
            }
            else if (me->HealthBelowPct(AI_MIN_HP) && who->GetEntry() == NPC_BLOODFANG_WORGEN)
                damage = 0;
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 1.0f)))
                    if (Creature* enemy = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 10.0f, true))
                        me->AI()->AttackStart(enemy);
                tSeek = urand(1000, 2000); // optimize cpu load, seeking only sometime between 1 and 2 seconds
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_sergeant_cleese_35839AI(creature);
    }
};

/* 35077 - QUEST: 14154 - By The Skin of His Teeth - START */
class npc_lord_darius_crowley_35077 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35077() : CreatureScript("npc_lord_darius_crowley_35077") { }

    enum eNpc
    {
        ACTION_START_EVENT = 101,
        SPELL_BY_THE_SKIN = 66914,
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == 14154)
            if (CAST_AI(npc_lord_darius_crowley_35077AI, creature->AI())->m_playerGUID == 0)
            {
                creature->AI()->SetGUID(player->GetGUID());
                creature->AI()->DoAction(ACTION_START_EVENT);
                creature->CastSpell(player, SPELL_BY_THE_SKIN, true);
            }
        return true;
    }

    struct npc_lord_darius_crowley_35077AI : public ScriptedAI
    {
        npc_lord_darius_crowley_35077AI(Creature* creature) : ScriptedAI(creature), m_summons(me) { Init(); }

        enum eQ14154
        {
            NPC_WORGEN_RUNT_C1 = 35188,
            NPC_WORGEN_RUNT_C2 = 35456,
            NPC_WORGEN_ALPHA_C1 = 35170, 
            NPC_WORGEN_ALPHA_C2 = 35167,

            Event120Secounds = 1,
            EventCheckPlayerIsAlive,
            EventSummonNextWave,
            EventHelpPlayer,
        };

        uint64 m_playerGUID;
        EventMap m_events;
        SummonList m_summons;
        uint32 m_phase;

        void Init()
        {
            m_events.Reset();
            m_playerGUID = 0;
            m_phase = 0;
            m_summons.DespawnAll();
        }

        void JustSummoned(Creature* summoned) override
        {
            m_summons.Summon(summoned);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            m_summons.Despawn(summon);
        }

        void DoAction(int32 /*action*/) override
        {
            m_phase = 1;
            m_events.ScheduleEvent(EventCheckPlayerIsAlive, 1000);
            m_events.ScheduleEvent(EventSummonNextWave, 1000);
            m_events.ScheduleEvent(Event120Secounds, 120000);
            m_events.ScheduleEvent(EventHelpPlayer, 250);
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void DamageTaken(Unit* attacker, uint32& /*damage*/) override
        {
            if (Creature* worgen = attacker->ToCreature())
                if (!me->IsInCombat())
                {
                } //   me->Attack(worgen, true);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                case Event120Secounds:
                    Init();
                    break;
                case EventCheckPlayerIsAlive: // check every sec player is alive
                    if (m_playerGUID && m_phase)
                        if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                            if (!player->IsInWorld() || !player->IsAlive())
                                Init();

                    m_events.ScheduleEvent(EventCheckPlayerIsAlive, 1000);
                    break;
                case EventSummonNextWave:
                {
                    for (int i = 0; i < 4; i++)
                    {
                        uint32 w1 = RAND(NPC_WORGEN_RUNT_C1, NPC_WORGEN_RUNT_C2, NPC_WORGEN_ALPHA_C1, NPC_WORGEN_ALPHA_C2);
                        uint32 w2 = RAND(NPC_WORGEN_RUNT_C1, NPC_WORGEN_RUNT_C2, NPC_WORGEN_ALPHA_C1, NPC_WORGEN_ALPHA_C2);
                        Creature* creature1 = me->SummonCreature(w1, -1610.39f, 1507.16f, 74.99f, 3.94f, TEMPSUMMON_TIMED_DESPAWN, 120000);
                        m_summons.Summon(creature1);
                        creature1->AI()->SetGUID(m_playerGUID);
                        creature1->AI()->DoAction(1);
                        Creature* creature2 = me->SummonCreature(w2, -1718.01f, 1516.81f, 55.40f, 4.6f, TEMPSUMMON_TIMED_DESPAWN, 120000);
                        m_summons.Summon(creature2);
                        creature2->AI()->SetGUID(m_playerGUID);
                        creature2->AI()->DoAction(2);
                    }

                    m_events.ScheduleEvent(EventSummonNextWave, 30000); // every 30 secounds one wave
                    break;
                }
                case EventHelpPlayer:
                {
                    if (!me->IsInCombat())
                    {
                        Creature* creature = NULL;
                        creature = me->FindNearestCreature(NPC_WORGEN_RUNT_C1, 5.0f);
                        if (!creature)
                            creature = me->FindNearestCreature(NPC_WORGEN_RUNT_C2, 5.0f);
                        if (!creature)
                            creature = me->FindNearestCreature(NPC_WORGEN_ALPHA_C1, 5.0f);
                        if (!creature)
                            creature = me->FindNearestCreature(NPC_WORGEN_ALPHA_C2, 5.0f);
                        if (creature)
                        {
                            me->Attack(creature, true);
                            // creature->Attack(me, true);
                        }
                    }

                    m_events.ScheduleEvent(EventHelpPlayer, 250);
                    break;
                }
                }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_darius_crowley_35077AI(creature);
    }
};

// 35124
class npc_tobias_mistmantle_35124 : public CreatureScript
{
public:
    npc_tobias_mistmantle_35124() : CreatureScript("npc_tobias_mistmantle_35124") { }

    struct npc_tobias_mistmantle_35124AI : public ScriptedAI
    {
        npc_tobias_mistmantle_35124AI(Creature* creature) : ScriptedAI(creature)  { }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_tobias_mistmantle_35124AI(creature);
    }
};

// 35188
class npc_worgen_runt_35188 : public CreatureScript
{
public:
    npc_worgen_runt_35188() : CreatureScript("npc_worgen_runt_35188") {}

    struct npc_worgen_runt_35188AI : public ScriptedAI
    {
        npc_worgen_runt_35188AI(Creature* creature) : ScriptedAI(creature) { Init(); }

        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;
        Position JumpW1[3];
        Position LandingW1[3];
        Position LandingW2[4];

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
            JumpW1[0] = Position(-1643.91f, 1482.96f, 63.22f, 3.95f);
            JumpW1[1] = Position(-1638.83f, 1478.07f, 65.36f, 3.84f);
            JumpW1[2] = Position(-1631.49f, 1475.02f, 65.64f, 3.84f);

            LandingW1[0] = Position(-1677.39f, 1455.52f, 52.29f, 4.06f);
            LandingW1[1] = Position(-1671.89f, 1449.27f, 52.29f, 3.84f);
            LandingW1[2] = Position(-1664.82f, 1443.62f, 52.29f, 3.84f);

            LandingW2[0] = Position(-1704.93f, 1469.07f, 52.29f, 5.34f);
            LandingW2[1] = Position(-1698.90f, 1472.92f, 52.29f, 5.41f);
            LandingW2[2] = Position(-1703.37f, 1470.66f, 52.29f, 5.49f);
            LandingW2[3] = Position(-1700.37f, 1473.32f, 52.29f, 5.41f);
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
            case 1:
            {
                m_events.ScheduleEvent(1, 500);
                DoWalk();
                break;
            }
            }



            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
                    if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_35188AI(creature);
    }
};

// 35456
class npc_worgen_runt_35456 : public CreatureScript
{
public:
    npc_worgen_runt_35456() : CreatureScript("npc_worgen_runt_35456") { }

    enum eNpc
    {
       
    };

    struct npc_worgen_runt_35456AI : public ScriptedAI
    {
        npc_worgen_runt_35456AI(Creature* creature) : ScriptedAI(creature) { Init(); }

        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;
        Position JumpW1[3];
        Position LandingW1[3];
        Position LandingW2[4];

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
            JumpW1[0] = Position(-1643.91f, 1482.96f, 63.22f, 3.95f);
            JumpW1[1] = Position(-1638.83f, 1478.07f, 65.36f, 3.84f);
            JumpW1[2] = Position(-1631.49f, 1475.02f, 65.64f, 3.84f);

            LandingW1[0] = Position(-1677.39f, 1455.52f, 52.29f, 4.06f);
            LandingW1[1] = Position(-1671.89f, 1449.27f, 52.29f, 3.84f);
            LandingW1[2] = Position(-1664.82f, 1443.62f, 52.29f, 3.84f);

            LandingW2[0] = Position(-1704.93f, 1469.07f, 52.29f, 5.34f);
            LandingW2[1] = Position(-1698.90f, 1472.92f, 52.29f, 5.41f);
            LandingW2[2] = Position(-1703.37f, 1470.66f, 52.29f, 5.49f);
            LandingW2[3] = Position(-1700.37f, 1473.32f, 52.29f, 5.41f);
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
            case 1:
            {
                m_events.ScheduleEvent(1, 500);
                DoWalk();
                break;
            }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
                    if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_runt_35456AI(creature);
    }
};

// 35170
class npc_worgen_alpha_35170 : public CreatureScript
{
public:
    npc_worgen_alpha_35170() : CreatureScript("npc_worgen_alpha_35170") {}

    struct npc_worgen_alpha_35170AI : public ScriptedAI
    {
        npc_worgen_alpha_35170AI(Creature* creature) : ScriptedAI(creature) { Init(); }

        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;
        Position JumpW1[3];
        Position LandingW1[3];
        Position LandingW2[4];

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
            JumpW1[0] = Position(-1643.91f, 1482.96f, 63.22f, 3.95f);
            JumpW1[1] = Position(-1638.83f, 1478.07f, 65.36f, 3.84f);
            JumpW1[2] = Position(-1631.49f, 1475.02f, 65.64f, 3.84f);

            LandingW1[0] = Position(-1677.39f, 1455.52f, 52.29f, 4.06f);
            LandingW1[1] = Position(-1671.89f, 1449.27f, 52.29f, 3.84f);
            LandingW1[2] = Position(-1664.82f, 1443.62f, 52.29f, 3.84f);

            LandingW2[0] = Position(-1704.93f, 1469.07f, 52.29f, 5.34f);
            LandingW2[1] = Position(-1698.90f, 1472.92f, 52.29f, 5.41f);
            LandingW2[2] = Position(-1703.37f, 1470.66f, 52.29f, 5.49f);
            LandingW2[3] = Position(-1700.37f, 1473.32f, 52.29f, 5.41f);
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
            case 1:
            {
                m_events.ScheduleEvent(1, 500);
                DoWalk();
                break;
            }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
                    if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_35170AI(creature);
    }
};

// 35167
class npc_worgen_alpha_35167 : public CreatureScript
{
public:
    npc_worgen_alpha_35167() : CreatureScript("npc_worgen_alpha_35167") {}

    struct npc_worgen_alpha_35167AI : public ScriptedAI
    {
        npc_worgen_alpha_35167AI(Creature* creature) : ScriptedAI(creature) { Init(); }

        uint64 m_playerGUID;
        EventMap m_events;
        uint32 m_phase;
        Position jump;
        Position JumpW1[3];
        Position LandingW1[3];
        Position LandingW2[4];

        void Init()
        {
            m_playerGUID = 0;
            m_phase = 0;
            JumpW1[0] = Position(-1643.91f, 1482.96f, 63.22f, 3.95f);
            JumpW1[1] = Position(-1638.83f, 1478.07f, 65.36f, 3.84f);
            JumpW1[2] = Position(-1631.49f, 1475.02f, 65.64f, 3.84f);

            LandingW1[0] = Position(-1677.39f, 1455.52f, 52.29f, 4.06f);
            LandingW1[1] = Position(-1671.89f, 1449.27f, 52.29f, 3.84f);
            LandingW1[2] = Position(-1664.82f, 1443.62f, 52.29f, 3.84f);

            LandingW2[0] = Position(-1704.93f, 1469.07f, 52.29f, 5.34f);
            LandingW2[1] = Position(-1698.90f, 1472.92f, 52.29f, 5.41f);
            LandingW2[2] = Position(-1703.37f, 1470.66f, 52.29f, 5.49f);
            LandingW2[3] = Position(-1700.37f, 1473.32f, 52.29f, 5.41f);
        }

        void DoAction(int32 action) override
        {
            if (m_playerGUID)
            {
                m_events.ScheduleEvent(1, 500);
                m_phase = action;
                me->SetSpeed(MOVE_RUN, frand(1.2f, 1.6f));
            }
        }

        void SetGUID(uint64 guid, int32 type = 0) override
        {
            m_playerGUID = guid;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE || type == EFFECT_MOTION_TYPE)
                m_phase = pointId;
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            uint32 eventId = m_events.ExecuteEvent();
            switch (eventId)
            {
            case 1:
            {
                m_events.ScheduleEvent(1, 500);
                DoWalk();
                break;
            }
            }

            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }

        void DoWalk()
        {
            switch (m_phase)
            {
            case 1:
            {
                m_phase = 3;
                uint8 rol = urand(0, 2);
                jump = JumpW1[rol];
                me->GetMotionMaster()->MovePoint(11 + rol, jump);
                break;
            }
            case 2:
                m_phase = 3;
                jump = Position(-1717.73f, 1486.27f, 57.23f, 5.45f);
                me->GetMotionMaster()->MovePoint(21, jump);
                break;
            case 11:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[0], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 12:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[1], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 13:
                m_phase = 4;
                me->GetMotionMaster()->MoveJump(LandingW1[2], frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            case 21:
            {
                m_phase = 5;
                uint8 rol = urand(0, 3);
                jump = LandingW2[rol];
                me->GetMotionMaster()->MoveJump(jump, frand(20.0f, 25.0f), frand(15.0f, 20.0f), 25);
                break;
            }
            case 25:
                m_phase = 6;
                if (m_playerGUID)
                    if (Player* player = ObjectAccessor::GetPlayer(*me, m_playerGUID))
                        if (player->IsInWorld() || player->IsAlive())
                        {
                            Position pos = player->GetNearPosition(frand(2.0f, 4.0f), frand(3.14f, 6.28f));
                            me->GetMotionMaster()->MovePoint(26, pos);
                        }
                break;
            case 26:
                m_phase = 7;
                me->SetHomePosition(me->GetPosition());
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_worgen_alpha_35167AI(creature);
    }
};

// 35112
class npc_king_genn_greymane_35112 : public CreatureScript
{
public:
    npc_king_genn_greymane_35112() : CreatureScript("npc_king_genn_greymane_35112") { }

    struct npc_king_genn_greymane_35112AI : public ScriptedAI
    {
        npc_king_genn_greymane_35112AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_genn_greymane_35112AI(creature);
    }
};

// 35115
class npc_lord_godfrey_35115 : public CreatureScript
{
public:
    npc_lord_godfrey_35115() : CreatureScript("npc_lord_godfrey_35115") { }

    struct npc_lord_godfrey_35115AI : public ScriptedAI
    {
        npc_lord_godfrey_35115AI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_godfrey_35115AI(creature);
    }
};

// 35118 showfight <> guard 34916
class npc_bloodfang_worgen_35118 : public CreatureScript
{
public:
    npc_bloodfang_worgen_35118() : CreatureScript("npc_bloodfang_worgen_35118") {}

    enum eNpc
    {
        EVENT_ENRAGE_COOLDOWN = 101,
        SPELL_ENRAGE = 8599,
    };

    struct npc_bloodfang_worgen_35118AI : public ScriptedAI
    {
        npc_bloodfang_worgen_35118AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        bool m_enrage;

        void Reset() override
        {
            m_enrage = false;
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (!m_enrage && me->GetHealthPct() < 50.0f)
            {
                me->CastSpell(me, SPELL_ENRAGE);
                m_enrage = true;
                m_events.ScheduleEvent(EVENT_ENRAGE_COOLDOWN, 20000);
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (Player* player = caster->ToPlayer())
            {
                if (player->GetQuestStatus(14276) == QUEST_STATUS_INCOMPLETE)
                    if (spell->Id == 56641)
                        player->KilledMonsterCredit(44175);

                if (player->GetQuestStatus(14281) == QUEST_STATUS_INCOMPLETE)
                    if (spell->Id == 5143)
                        player->KilledMonsterCredit(44175);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENRAGE_COOLDOWN:
                    {
                        m_enrage = false;
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_worgen_35118AI(creature);
    }
};
/* QUEST - 14154 - By The Skin of His Teeth - END */

/* Phase 4 - QUEST - 14159 - The Rebel Lord's Arsenal - START */

// 35369
class npc_josiah_avery_35369 : public CreatureScript
{
public:
    npc_josiah_avery_35369() : CreatureScript("npc_josiah_avery_35369") {}

    enum eNpc
    {
        QUEST_THE_REBEL_LORDS_ARSENAL = 14159,
        NPC_JOSIAH_AVERY_P4 = 35370,
        NPC_JOSAIH_AVERY_TRIGGER = 50415,
        SPELL_WORGEN_BITE = 72870,
        SPELL_PHASE_QUEST_ZONE_SPECIFIC_01 = 59073,
        SPELL_SUMMON_JOSIAH_AVERY = 67350,
        SPELL_FORCE_CAST_SUMMON_JOSIAH = 67352,
        DELAY_SAY_JOSIAH_AVERY = 20000,
        EVENT_SAY_JOSIAH_AVERY = 101,
        ACTION_START_ANIM = 102,
        PLAYER_GUID = 99999,
    };

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 opt) override
    {
        if (quest->GetQuestId() == QUEST_THE_REBEL_LORDS_ARSENAL)
        {
            creature->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
            creature->AddAura(SPELL_WORGEN_BITE, player);
            player->RemoveAura(SPELL_PHASE_QUEST_ZONE_SPECIFIC_01);
            creature->CastSpell(creature, SPELL_FORCE_CAST_SUMMON_JOSIAH, true);
            player->SetPhaseMask(4, true);
        }
        return true;
    }

    struct npc_josiah_avery_35369AI : public ScriptedAI
    {
        npc_josiah_avery_35369AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        uint32 m_currentSayCounter; // Current Say
        uint64 m_playerGUID;
        uint64 m_badAveryGUID;
        uint64 m_triggerGUID;

        // Evade or Respawn
        void Reset() override
        {
            m_playerGUID = NULL;
            m_badAveryGUID = NULL;
            m_triggerGUID = NULL;
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_SAY_JOSIAH_AVERY, DELAY_SAY_JOSIAH_AVERY);
            m_currentSayCounter = 0;
        }

        void JustSummoned(Creature* summon) override
        { 
            switch (summon->GetEntry())
            {
                case NPC_JOSIAH_AVERY_P4:
                {
                    summon->SetPhaseMask(4, true);
                    summon->RemoveAllAuras();
                    m_badAveryGUID = summon->GetGUID();
                    break;
                }
                case NPC_JOSAIH_AVERY_TRIGGER:
                {
                    summon->SetPhaseMask(6, true);
                    m_triggerGUID = summon->GetGUID();
                    break;
                }
            }
            if (PLAYER_GUID && NPC_JOSIAH_AVERY_P4 && NPC_JOSAIH_AVERY_TRIGGER)
                if (Creature* trigger = sObjectAccessor->GetCreature(*me, m_triggerGUID))
                {
                    trigger->AI()->SetGUID(m_playerGUID, PLAYER_GUID);
                    trigger->AI()->SetGUID(m_badAveryGUID, NPC_JOSIAH_AVERY_P4);
                    trigger->AI()->DoAction(ACTION_START_ANIM);
                }
        }

        void SetGUID(uint64 guid, int32 id) override
        { 
            switch (id)
            {
                case PLAYER_GUID:
                {
                    m_playerGUID = guid;
                    break;
                }
            }
        }

        //Timed events
        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SAY_JOSIAH_AVERY:
                    {
                        m_currentSayCounter += 1;
                        if (m_currentSayCounter > 5)
                            m_currentSayCounter = 1;

                        std::list<Player*> pList = GetListOfPlayersNearAndIndoorsAndWithQuest();
                        TalkToGroup(pList, m_currentSayCounter);

                        m_events.ScheduleEvent(EVENT_SAY_JOSIAH_AVERY, DELAY_SAY_JOSIAH_AVERY);
                        break;
                    }
                }
            }
        }

        std::list<Player*> GetListOfPlayersNearAndIndoorsAndWithQuest()
        {
            std::list<Player*> pList = me->FindNearestPlayers(20.0f);
            while (DeleteWrongPlayer(pList)) {}
            return pList;
        }

        bool DeleteWrongPlayer(std::list<Player*> &pList)
        {
            if (pList.empty())
                return false;

            for (std::list<Player*>::const_iterator itr = pList.begin(); itr != pList.end(); itr++)
            {
                Player* player = (*itr);
                if (!player->GetMap()->IsIndoors(player))
                {
                    pList.remove(player);
                    return true;
                }
                if (player->GetQuestStatus(QUEST_THE_REBEL_LORDS_ARSENAL) != QUEST_STATUS_COMPLETE)
                {
                    pList.remove(player);
                    return true;
                }
            }
            return false;
        }

        void TalkToGroup(std::list<Player*> pList, uint8 groupId)
        {
            if (pList.empty())
                return;

            for (std::list<Player*>::const_iterator itr = pList.begin(); itr != pList.end(); itr++)
                Talk(groupId, (*itr));
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_35369AI(creature);
    }
};

// 50415
class npc_josiah_avery_trigger_50415 : public CreatureScript
{
public:
    npc_josiah_avery_trigger_50415() : CreatureScript("npc_josiah_avery_trigger_50415") {}

    enum eNpc
    {
        NPC_LORNA_CROWLEY_P4 = 35378,
        NPC_JOSIAH_AVERY_P4 = 35370,
        SPELL_COSMETIC_COMBAT_ATTACK = 69873,
        SPELL_GET_SHOT = 67349,
        SPELL_SHOOT = 6660,
        SPELL_WORGEN_BITE = 72870,
        SAY_JOSAIH_AVERY_TRIGGER = 1,
        PLAYER_GUID = 99999,
        ACTION_START_ANIM = 102,
        EVENTS_ANIM_1,
        EVENTS_ANIM_2,
        EVENTS_ANIM_3,
        EVENTS_ANIM_4,
        EVENTS_ANIM_5,
        EVENTS_START_ANIM,
    };

    struct npc_josiah_avery_trigger_50415AI : public ScriptedAI
    {
        npc_josiah_avery_trigger_50415AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        uint64 m_playerGUID;
        uint64 m_badAveryGUID;
        uint64 m_lornaGUID;

        void Reset() override
        {
            m_playerGUID = NULL;
            m_badAveryGUID = NULL;
            m_lornaGUID = NULL;
            m_events.Reset();
            Position pos = Position(-1792.37f, 1427.35f, 12.46f, 3.152f);
            me->MovePosition(pos, 0, 0);
            me->SetDisplayId(11686);
        }

        void SetGUID(uint64 guid, int32 id) override
        {
            switch (id)
            {
                case PLAYER_GUID:
                {
                    m_playerGUID = guid;
                    break;
                }
                case NPC_JOSIAH_AVERY_P4:
                {
                    m_badAveryGUID = guid;
                    break;
                }
            }
        }

        void DoAction(int32 param) override
        {
            switch (param)
            {
                case ACTION_START_ANIM:
                {
                    if (Creature* lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY_P4, 60.0f, true))
                        m_lornaGUID = lorna->GetGUID();

                    m_events.ScheduleEvent(EVENTS_START_ANIM, 25);
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENTS_START_ANIM:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            me->AI()->Talk(SAY_JOSAIH_AVERY_TRIGGER, player); // Tell Player they have been bitten
                        m_events.ScheduleEvent(EVENTS_ANIM_1, 200);
                        break;
                    }
                    case EVENTS_ANIM_1:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* badAvery = sObjectAccessor->GetCreature(*me, m_badAveryGUID))
                            {
                                badAvery->SetOrientation(badAvery->GetAngle(player)); // Face Player
                                badAvery->CastSpell(player, SPELL_COSMETIC_COMBAT_ATTACK, true); // Do Cosmetic Attack
                                player->GetMotionMaster()->MoveKnockTo(-1791.94f, 1427.29f, 12.4584f, 22.0f, 8.0f, uint32(m_playerGUID));
                                badAvery->getThreatManager().resetAllAggro();
                            }
                        m_events.ScheduleEvent(EVENTS_ANIM_2, 1200);
                        break;
                    }
                    case EVENTS_ANIM_2:
                    {
                        if (Creature* badAvery = sObjectAccessor->GetCreature(*me, m_badAveryGUID))
                            badAvery->GetMotionMaster()->MoveJump(-1791.94f, 1427.29f, 12.4584f, 18.0f, 7.0f);
                        m_events.ScheduleEvent(EVENTS_ANIM_3, 600);
                        break;
                    }
                    case EVENTS_ANIM_3:
                    {
                        if (Creature* badAvery = sObjectAccessor->GetCreature(*me, m_badAveryGUID))
                            if (Creature* lorna = sObjectAccessor->GetCreature(*me, m_lornaGUID))
                                lorna->CastSpell(badAvery, SPELL_SHOOT, true);
                        m_events.ScheduleEvent(EVENTS_ANIM_4, 200);
                        break;
                    }
                    case EVENTS_ANIM_4:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* badAvery = sObjectAccessor->GetCreature(*me, m_badAveryGUID))
                            {
                                badAvery->CastSpell(badAvery, SPELL_GET_SHOT, true);
                                badAvery->SetDeathState(JUST_DIED);
                                player->SaveToDB();
                                badAvery->DespawnOrUnsummon(1000);
                                me->DespawnOrUnsummon(1000);
                            }
                        m_events.ScheduleEvent(EVENTS_ANIM_5, 5000);
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_josiah_avery_trigger_50415AI(creature);
    }
};

// 35378
class npc_lorna_crowley_35378 : public CreatureScript
{
public:
    npc_lorna_crowley_35378() : CreatureScript("npc_lorna_crowley_35378") {}

    enum eNpc
    {
        QUEST_FROM_THE_SHADOWS = 14204,
        NPC_GILNEAN_MASTIFF = 35631,
        SPELL_SUMMON_GILNEAN_MASTIFF = 67807,
        SAY_LORNA_CROWLEY_P4 = 0,
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
        {
            if (Pet* pet = player->GetPet())
                player->RemovePet(pet, PET_SAVE_AS_CURRENT, true);
            player->CastSpell(player, SPELL_SUMMON_GILNEAN_MASTIFF);
            creature->AI()->Talk(SAY_LORNA_CROWLEY_P4);
        }
        return true;
    }
};

// 35631
class npc_gilnean_mastiff_35631 : public CreatureScript
{
public:
    npc_gilnean_mastiff_35631() : CreatureScript("npc_gilnean_mastiff_35631") { }

    enum eNpc
    {
        QUEST_FROM_THE_SHADOWS = 14204,
        NPC_BLOODFANG_LURKER = 35463,
        SPELL_SHADOWSTALKER_STEALTH = 5916,
        SPELL_ATTACK_LURKER = 67805,
        EVENT_CHECK_QUEST_REWARDED = 101,
    };

    struct npc_gilnean_mastiff_35631AI : public ScriptedAI
    {
        npc_gilnean_mastiff_35631AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        EventMap m_events;
        uint64 m_playerGUID;
        uint64 m_lurkerGUID;

        void Initialize()
        {
            m_playerGUID = NULL;
            m_lurkerGUID = NULL;
        }

        void Reset() override
        {
            if (CharmInfo* info = me->GetCharmInfo())
            {
                info->InitEmptyActionBar(false);
                info->SetActionBar(0, SPELL_ATTACK_LURKER, ACT_PASSIVE);
                me->SetReactState(REACT_DEFENSIVE);
                info->SetIsFollowing(true);
            }
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CHECK_QUEST_REWARDED, 10000);
        }

        void IsSummonedBy(Unit* summoner) override
        { 
            if (Player* player = summoner->ToPlayer())
                m_playerGUID = player->GetGUID();
        }

        void SpellHitTarget(Unit* mastiff, SpellInfo const* cSpell) override
        {
            
        }

        void JustDied(Unit* /*killer*/) override // Otherwise, player is stuck with pet corpse they cannot remove from world
        {
            me->DespawnOrUnsummon(1);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_QUEST_REWARDED:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (player->GetQuestStatus(QUEST_FROM_THE_SHADOWS) != QUEST_STATUS_INCOMPLETE)
                                me->DespawnOrUnsummon(1);

                        m_events.ScheduleEvent(EVENT_CHECK_QUEST_REWARDED, 10000);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_mastiff_35631AI(creature);
    }
};

// 35463
class npc_bloodfang_lurker_35463 : public CreatureScript
{
public:
    npc_bloodfang_lurker_35463() : CreatureScript("npc_bloodfang_lurker_35463") {}

    enum eNpc
    {
        SPELL_SHADOWSTALKER_STEALTH = 5916,
        SPELL_ENRAGE = 8599,
        COOLDOWN_ENRAGE = 121000,
    };

    struct npc_bloodfang_lurker_35463AI : public ScriptedAI
    {
        npc_bloodfang_lurker_35463AI(Creature* creature) : ScriptedAI(creature) {}

        uint32 tEnrage, tSeek;
        bool willCastEnrage;

        void Reset() override
        {
            tEnrage = 0;
            willCastEnrage = urand(0, 1);
            tSeek = urand(5000, 10000);
            DoCast(me, SPELL_SHADOWSTALKER_STEALTH);
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSeek <= diff)
            {
                if ((me->IsAlive()) && (!me->IsInCombat() && (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 2.0f)))
                    if (Player* player = me->SelectNearestPlayer(2.0f))
                    {
                        if (!player->IsInCombat())
                        {
                            me->AI()->AttackStart(player);
                            tSeek = urand(5000, 10000);
                        }
                    }
            }
            else tSeek -= diff;

            if (!UpdateVictim())
                return;

            if (tEnrage <= diff && willCastEnrage && me->GetHealthPct() <= 30)
            {
                me->MonsterTextEmote(-106, 0);
                DoCast(me, SPELL_ENRAGE);
                tEnrage = COOLDOWN_ENRAGE;
            }
            else
                tEnrage -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_lurker_35463AI(creature);
    }
};

// 35504
class npc_gilnean_city_guard_35504 : public CreatureScript
{
public:
    npc_gilnean_city_guard_35504() : CreatureScript("npc_gilnean_city_guard_35504") {}

    enum eNpc
    {
        NPC_RAMPAGING_WORGEN_35660 = 35660,
        NPC_BLOODFANG_RIPPER_35505 = 35505,

        EVENT_CHECK_SHOWFIGHT = 101,

        MOVE_TO_HOMEPOSITION = 101,
    };

    struct npc_gilnean_city_guard_35504AI : public ScriptedAI
    {
        npc_gilnean_city_guard_35504AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        EventMap m_events;
        float    m_minHealthPct;
        std::list<uint32> m_worgenList;

        void Initialize()
        {
            m_worgenList.clear();
            m_worgenList.push_back(NPC_RAMPAGING_WORGEN_35660);
            m_worgenList.push_back(NPC_BLOODFANG_RIPPER_35505);
            m_minHealthPct = frand(30.0f, 85.0f);
        }

        void Reset() override
        {
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 1000);
        }

        void DamageTaken(Unit* who, uint32 &Damage) override
        {
            if (who->ToCreature())
                if (me->GetHealthPct() < m_minHealthPct)
                    Damage = 0;
                else
                    Damage = 1;
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE && pointId == MOVE_TO_HOMEPOSITION)
                m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 25);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_SHOWFIGHT:
                    {
                        if (me->IsDead() || me->IsInCombat())
                        {
                            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                            return;
                        }

                        if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 10.0f)
                        {
                            me->GetMotionMaster()->MovePoint(MOVE_TO_HOMEPOSITION, me->GetHomePosition());
                            return;
                        }

                        if (Creature* worgen = me->FindNearestCreature(m_worgenList, 10.0f))
                        {
                            me->Attack(worgen, true);
                            worgen->Attack(me, true);
                            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                            return;
                        }

                        m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_gilnean_city_guard_35504AI(creature);
    }
};

// 35550
class npc_king_genn_greymane_35550 : public CreatureScript
{
public:
    npc_king_genn_greymane_35550() : CreatureScript("npc_king_genn_greymane_35550") {}

    enum eNpc
    {
        QUEST_SAVE_KRENNAN_ARANAS = 14293,
        NPC_BLOODFANG_RIPPER_P4 = 35505,
        NPC_GRAYMANE_HORSE = 35905,
        SPELL_FORCECAST_SUMMON_GRAYMANE_HORSE = 68232,
        SPELL_GENERIC_QUEST_INVISIBILITY_DETECTION_1 = 49416,
        SAY_KING_GENN_GREYMANE = 0,
        MAX_SUMMONED_RIPPER = 20,
        PLAYER_GUID = 99999,
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            creature->GetAI()->SetGUID(player->GetGUID(), PLAYER_GUID);
            player->CastSpell(player, SPELL_GENERIC_QUEST_INVISIBILITY_DETECTION_1);
            player->CastSpell(player, SPELL_FORCECAST_SUMMON_GRAYMANE_HORSE, true);
        }
        return true;
    }

    struct npc_king_genn_greymane_35550AI : public ScriptedAI
    {
        npc_king_genn_greymane_35550AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        uint32 tSummon, tSay;
        bool EventActive, RunOnce;
        uint32 m_counter;
        uint64 m_playerGUID;

        void Initialize()
        {
            m_counter = 0;
            m_playerGUID = NULL;
        }

        void Reset() override
        {
            tSay = urand(10000, 20000);
            tSummon = urand(3000, 5000); // How often we spawn
        }

        void JustSummoned(Creature* summoned) override
        {
            switch (summoned->GetEntry())
            {
                case NPC_BLOODFANG_RIPPER_P4:
                {
                    m_counter += 1;
                    summoned->GetDefaultMovementType();
                    summoned->SetReactState(REACT_AGGRESSIVE);
                    break;
                }
            }
        }

        void SummonedCreatureDespawn(Creature* summon) override
        { 
            if (summon->GetEntry() == NPC_BLOODFANG_RIPPER_P4 && m_counter)
                m_counter -= 1;
        }

        void SetGUID(uint64 guid, int32 id) override
        { 
            switch (id)
            {
                case PLAYER_GUID:
                {
                    m_playerGUID = guid;
                    break;
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (tSay <= diff) // Time for next spawn wave
            {
                Talk(SAY_KING_GENN_GREYMANE);
                tSay = urand(10000, 20000);
            }
            else tSay -= diff;

            if (tSummon <= diff) // Time for next spawn wave
            {
                SummonNextWave(); // Activate next spawn wave
                tSummon = urand(3000, 5000); // Reset our spawn timer
            }
            else tSummon -= diff;
        }

        void SummonNextWave()
        {
            switch (urand(1, 4))
            {
            case (1) :
                for (int i = 0; i < 5; i++)
                    if (m_counter < MAX_SUMMONED_RIPPER)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                break;
            case (2) :
                for (int i = 0; i < 5; i++)
                    if (m_counter < MAX_SUMMONED_RIPPER)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1756.30f + irand(-15, 15), 1380.61f + irand(-15, 15), 19.7652f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                break;
            case (3) :
                for (int i = 0; i < 5; i++)
                    if (m_counter < MAX_SUMMONED_RIPPER)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1739.84f + irand(-15, 15), 1384.87f + irand(-15, 15), 19.841f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                break;
            case (4) :
                for (int i = 0; i < 5; i++)
                    if (m_counter < MAX_SUMMONED_RIPPER)
                        me->SummonCreature(NPC_BLOODFANG_RIPPER_P4, -1781.173f + irand(-15, 15), 1372.90f + irand(-15, 15), 19.7803f, urand(0, 6), TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 15000);
                break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_genn_greymane_35550AI(creature);
    }
};

// 35905
class npc_king_greymanes_horse_35905 : public CreatureScript
{
public:
    npc_king_greymanes_horse_35905() : CreatureScript("npc_king_greymanes_horse_35905") { }

    enum eHorse
    {
        QUEST_SAVE_KRENNAN_ARANAS = 14293,
        NPC_KRENNAN_ARANAS_TREE = 35753,
        NPC_KRENNAN_ARANAS = 35907,
        SPELL_RESCUE_KRENNAN = 68219,
        SPELL_GENERIC_QUEST_INVISIBILITY_DETECTION_1 = 49416,
        PLAYER_GUID = 99999,
        SAY_KRENNAN_TREE_HELP = 0,
        SAY_HORSE_HOW_DO_HELP = 1,
        SAY_KRENNAN_HORSE_THANKS = 2,
        EVENT_SAY_KRENNAN_HELP = 101,   // krennan 35753 in tree
        EVENT_STARTING_RESCUE_PART2,
        EVENT_SAY_KRENNAN_THANKS, 
    };

    struct npc_king_greymanes_horse_35905AI : public npc_escortAI
    {
        npc_king_greymanes_horse_35905AI(Creature* creature) : npc_escortAI(creature) {}

        EventMap m_events;
        uint64 m_playerGUID;
        uint64 m_krennanHorseGUID;
        uint64 m_krennanTreeGUID;

        void AttackStart(Unit* /*who*/) override {}
        void EnterCombat(Unit* /*who*/) override {}
        void OnCharmed(bool /*apply*/) override { }
        void EnterEvadeMode() override {}

        void Reset() override
        {
            m_events.Reset();
            m_playerGUID = NULL; 
            m_krennanHorseGUID = NULL;
            m_krennanTreeGUID = NULL;
            me->GetMotionMaster()->MoveIdle();
            m_events.ScheduleEvent(EVENT_SAY_KRENNAN_HELP, 500);
        }

        void PassengerBoarded(Unit* who, int8 seatId, bool apply) override
        {
            if (apply)
            {
                if (Player* player = who->ToPlayer())
                {
                    if (!m_playerGUID)
                    {
                        m_playerGUID = player->GetGUID();
                        Start(false, true, m_playerGUID);
                        player->SetClientControl(me, false);
                    }
                }
                else if (who->GetEntry() == NPC_KRENNAN_ARANAS && !m_krennanHorseGUID)
                {
                    m_krennanHorseGUID = who->GetGUID();
                    if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                        player->RemoveAura(SPELL_GENERIC_QUEST_INVISIBILITY_DETECTION_1);
                    m_events.ScheduleEvent(EVENT_STARTING_RESCUE_PART2, 400);                   
                }
            }
            else if (seatId == 1)
            {
                m_events.ScheduleEvent(EVENT_SAY_KRENNAN_THANKS, 25);
            }
        }

        void WaypointReached(uint32 point) override
        {
            switch (point)
            {
                case 5:
                {
                    if (me->GetVehicleKit()->HasEmptySeat(1))
                    {
                        SetEscortPaused(true);
                        me->GetMotionMaster()->MoveJump(-1679.089f, 1348.42f, 15.31f, 25.0f, 15.0f);
                        if (Player* player = GetPlayerForEscort())
                        {
                            Talk(SAY_HORSE_HOW_DO_HELP, player);
                            player->SetClientControl(me, true);
                        }
                    }
                    break;
                }
                case 6:
                {
                    if (Player* player = GetPlayerForEscort())
                        player->SetClientControl(me, false);
                    break;
                }
                case 12:
                {
                    if (Player* player = GetPlayerForEscort())
                        player->ExitVehicle();
                    break;
                }
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
            {
                player->FailQuest(QUEST_SAVE_KRENNAN_ARANAS);
                player->RemoveAura(SPELL_GENERIC_QUEST_INVISIBILITY_DETECTION_1);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);           

            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SAY_KRENNAN_HELP:
                    {
                        if (!m_krennanTreeGUID)
                            if (Creature* krennan = me->FindNearestCreature(NPC_KRENNAN_ARANAS_TREE, 100.0f, true))
                                m_krennanTreeGUID = krennan->GetGUID();

                        if (!m_krennanHorseGUID)
                        {
                            if (Creature* krennan = sObjectAccessor->GetCreature(*me, m_krennanTreeGUID))
                                if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                                {
                                    krennan->AI()->Talk(SAY_KRENNAN_TREE_HELP, player);
                                    m_events.ScheduleEvent(EVENT_SAY_KRENNAN_HELP, urand(6000, 9000));
                                    break;
                                }
                            m_events.ScheduleEvent(EVENT_SAY_KRENNAN_HELP, 500);
                        }
                        break;
                    }
                    case EVENT_STARTING_RESCUE_PART2:
                    {
                        SetEscortPaused(false);
                        break;
                    }
                    case EVENT_SAY_KRENNAN_THANKS:
                    {
                        if (Creature* krennan = sObjectAccessor->GetCreature(*me, m_krennanHorseGUID))
                        {
                            krennan->AI()->Talk(0);
                            krennan->NearTeleportTo(-1771.03f, 1433.41f, 19.85f, 3.598f);
                            krennan->GetAI()->DoAction(0);
                        }
                        me->DespawnOrUnsummon(25);
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_king_greymanes_horse_35905AI(creature);
    }
};

// 35907
class npc_krennan_aranas_35907 : public CreatureScript
{
public:
    npc_krennan_aranas_35907() : CreatureScript("npc_krennan_aranas_35907") { }

    enum eNpc
    {
        NPC_GREYMANE_HORSE_P4 = 35905,
        SPELL_RIDE_BUNNY_SEAT2 = 84275,
    };

    struct npc_krennan_aranas_35907AI : public ScriptedAI
    {
        npc_krennan_aranas_35907AI(Creature* creature) : ScriptedAI(creature) { }

        void AttackStart(Unit* /*who*/) override {}
        void EnterCombat(Unit* /*who*/) override {}
        void EnterEvadeMode() override {}

        void IsSummonedBy(Unit* summoner) override
        { 
            if (Creature* horse = me->FindNearestCreature(NPC_GREYMANE_HORSE_P4, 20.0f))
                DoCast(horse, SPELL_RIDE_BUNNY_SEAT2, true);
        }

        void DoAction(int32 /*param*/) override
        { 
            me->DespawnOrUnsummon(8000);
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_krennan_aranas_35907AI(creature);
    }
};

// 35914
class npc_commandeered_cannon_35914 : public CreatureScript
{
public:
    npc_commandeered_cannon_35914() : CreatureScript("npc_commandeered_cannon_35914") { }

    enum eNpc
    {
        NPC_BLOODFANG_WORGEN = 35118,
        SPELL_CANNON_FIRE = 68235,
        ACTION_STARTING_EVENT = 101,
        EVENT_STARTING_EVENT,
        EVENT_STARTING_DO_FIRE,
    };

    struct npc_commandeered_cannon_35914AI : public ScriptedAI
    {
        npc_commandeered_cannon_35914AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;

        void Reset() override
        {
            m_events.Reset();
        }

        void JustSummoned(Creature* summon) override
        {
            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
        }

        void DoAction(int32 param) override
        {
            switch (param)
            {
                case ACTION_STARTING_EVENT:
                {
                    m_events.ScheduleEvent(EVENT_STARTING_EVENT, 25);
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_STARTING_EVENT:
                    {
                        for (int i = 0; i < 12; i++)
                        {
                            me->SummonCreature(NPC_BLOODFANG_WORGEN, -1757.65f + irand(-6, 6), 1384.01f + irand(-6, 6), 19.872f, urand(0, 6), TEMPSUMMON_TIMED_DESPAWN, 5000);
                        }
                        m_events.ScheduleEvent(EVENT_STARTING_DO_FIRE, 400);
                        break;
                    }
                    case EVENT_STARTING_DO_FIRE:
                    {
                        if (Creature* Worgen = me->FindNearestCreature(NPC_BLOODFANG_WORGEN, 50.0f, true))
                            me->CastSpell(Worgen, SPELL_CANNON_FIRE, true);
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_commandeered_cannon_35914AI(creature);
    }
};

// 35906
class npc_lord_godfrey_35906 : public CreatureScript
{
public:
    npc_lord_godfrey_35906() : CreatureScript("npc_lord_godfrey_35906") { }

    enum eNpc
    {
        QUEST_SAVE_KRENNAN_ARANAS = 14293,
        NPC_COMMANDEERED_CANNON = 35914,
        SPELL_WORGEN_BITE = 72870,         // phase 4
        SPELL_INFECTED_BITE = 72872,       // phase 8
        SPELL_HIDEOUS_BITE_WOUND = 76642,  // phase 1024
        SPELL_GILNEAS_CANNON_CAMERA = 93555,
        SAY_LORD_GODFREY_P4 = 0,
        ACTION_STARTING_EVENT = 101,
    };

    bool OnQuestReward(Player* player, Creature* godfrey, Quest const* quest, uint32 opt)
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            godfrey->AI()->Talk(SAY_LORD_GODFREY_P4);
            player->RemoveAura(SPELL_WORGEN_BITE);
            godfrey->AddAura(SPELL_INFECTED_BITE, player);
            player->CastSpell(player, SPELL_GILNEAS_CANNON_CAMERA);
            player->SaveToDB();
            if (Creature* cannon = GetClosestCreatureWithEntry(godfrey, NPC_COMMANDEERED_CANNON, 50.0f))
                cannon->GetAI()->DoAction(ACTION_STARTING_EVENT);
        }
        return true;
    }
};

/* Phase 8 - START */

// 35552
class npc_lord_darius_crowley_35552 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35552() : CreatureScript("npc_lord_darius_crowley_35552") { }

    enum eNpc
    {
        QUEST_SACRIFICES = 14212,
        NPC_KING_GREYMANE = 35911,
        NPC_PRINCE_GREYMANE = 35551,
        NPC_CROWLEY = 35230,
        NPC_CROWLEYS_HORSE = 35231,
        PLAYER_GUID = 99999,
        EVENT_START_TALK_PARTY = 101,
        EVENT_TALK_PARTY_1,
        EVENT_TALK_PARTY_2,
        EVENT_TALK_PARTY_3,
        EVENT_TALK_PARTY_4,
        EVENT_TALK_PARTY_5,
        EVENT_TALK_PARTY_6,
        EVENT_RESET_TALK_PARTY,
        EVENT_REBEL_VALOR,

    };

    struct npc_lord_darius_crowley_35552AI : public ScriptedAI
    {
        npc_lord_darius_crowley_35552AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        EventMap m_events;
        uint64 m_kingGUID;
        uint64 m_princeGUID;
        uint64 m_playerGUID;

        void Initialize()
        {
            m_kingGUID = NULL;
            m_princeGUID = NULL;
            m_playerGUID = NULL;
        }

        void Reset() override
        {
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_START_TALK_PARTY, 1000);
            m_events.ScheduleEvent(EVENT_REBEL_VALOR, 2500);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_TALK_PARTY:
                    {
                        if (!m_kingGUID)
                            if (Creature* king = me->FindNearestCreature(NPC_KING_GREYMANE, 15.0f))
                                m_kingGUID = king->GetGUID();
                        if (!m_princeGUID)
                            if (Creature* prince = me->FindNearestCreature(NPC_PRINCE_GREYMANE, 15.0f))
                                m_princeGUID = prince->GetGUID();
                        if (Player* player = me->FindNearestPlayer(15.0f))
                        {
                            m_playerGUID = player->GetGUID();
                            m_events.ScheduleEvent(EVENT_TALK_PARTY_1, 1000);
                            m_events.ScheduleEvent(EVENT_RESET_TALK_PARTY, 60000);
                            break;
                        }
                        m_events.ScheduleEvent(EVENT_START_TALK_PARTY, 1000);
                        break;
                    }
                    case EVENT_RESET_TALK_PARTY:
                    {
                        Reset();
                        break;
                    }
                    case EVENT_TALK_PARTY_1:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* king = sObjectAccessor->GetCreature(*me, m_kingGUID))
                                king->AI()->Talk(0, player);
                        m_events.ScheduleEvent(EVENT_TALK_PARTY_2, 9000);
                        break;
                    }
                    case EVENT_TALK_PARTY_2:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            Talk(0, player);
                        m_events.ScheduleEvent(EVENT_TALK_PARTY_3, 9000);
                        break;
                    }
                    case EVENT_TALK_PARTY_3:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* prince = sObjectAccessor->GetCreature(*me, m_princeGUID))
                                prince->AI()->Talk(0, player);
                        m_events.ScheduleEvent(EVENT_TALK_PARTY_4, 6000);
                        break;
                    }
                    case EVENT_TALK_PARTY_4:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            Talk(1, player);
                        m_events.ScheduleEvent(EVENT_TALK_PARTY_5, 9000);
                        break;
                    }
                    case EVENT_TALK_PARTY_5:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            Talk(2, player);
                        m_events.ScheduleEvent(EVENT_TALK_PARTY_6, 8000);
                        break;
                    }
                    case EVENT_TALK_PARTY_6:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* king = sObjectAccessor->GetCreature(*me, m_kingGUID))
                                king->AI()->Talk(2, player);
                        m_events.RescheduleEvent(EVENT_RESET_TALK_PARTY, urand(30000, 45000));
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lord_darius_crowley_35552AI(creature);
    }
};

// 56685
class spell_csa_dummy_effect_56685 : public SpellScriptLoader
{
public:
    spell_csa_dummy_effect_56685() : SpellScriptLoader("spell_csa_dummy_effect_56685") { }

    enum eSpell
    {
        QUEST_SACRIFICES = 14212,
        SPELL_SUMMON_CROWLEYS_HORSE = 67001,
        NPC_CROWLEY = 35230,
        NPC_CROWLEYS_HORSE = 35231,
    };

    class spell_csa_dummy_effect_56685_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_csa_dummy_effect_56685_SpellScript);

        bool Load() override
        {
            if (Player* player = GetCaster()->ToPlayer())
                if (player->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                    return true;

            return false;
        }

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Player* player = GetCaster()->ToPlayer())
                if (Creature* horse = player->SummonCreature(NPC_CROWLEYS_HORSE, player->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 180000))
                    if (Creature* crowley = horse->SummonCreature(NPC_CROWLEY, player->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 180000))
                    {
                        player->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 1, horse, false);
                        crowley->CastCustomSpell(VEHICLE_SPELL_RIDE_HARDCODED, SPELLVALUE_BASE_POINT0, 2, horse, false);
                    }
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_csa_dummy_effect_56685_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_csa_dummy_effect_56685_SpellScript();
    }
};

// 35231
class npc_crowley_horse_35231 : public CreatureScript
{
public:
    npc_crowley_horse_35231() : CreatureScript("npc_crowley_horse_35231") {}

    enum eNpc
    {
        QUEST_SACRIFICES = 14212,
        NPC_LORD_DARIUS_CROWLEY = 35552,
        NPC_DARIUS_CROWLEY = 35230,
        NPC_BLOODFANG_STALKER = 35229,
        SPELL_CROWLEY_FORCE_SUMMON = 67004,
        SPELL_RIDE_HORSE = 43671,
        SPELL_THROW_TORCH = 67063,
        PLAYER_GUID = 99999,
        SAY_CROWLEY_HORSE_1 = 0,
        SAY_CROWLEY_HORSE_2 = 1,
        MOVE_START_POSITION = 102,
        EVENT_CROWLEY_SAY = 101,
        EVENT_MOVE_TO_START,
        EVENT_JUMP_ON_BRIDGE,
        EVENT_CAST_TORCH,
    };

    struct npc_crowley_horse_35231AI : public ScriptedAI
    {
        npc_crowley_horse_35231AI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_events;
        uint64 m_playerGUID;
        uint64 m_dariusGUID;
        uint8  m_movePart;

        void AttackStart(Unit* /*who*/) override {}
        void EnterCombat(Unit* /*who*/) override {}
        void EnterEvadeMode() override {}

        void Reset() override
        {
            m_playerGUID = NULL;
            m_dariusGUID = NULL;
            m_movePart = 0;
        }

        void IsSummonedBy(Unit* summoner) override
        {
            if (Player* player = summoner->ToPlayer())
            {
                m_playerGUID = summoner->GetGUID();
            }
        }

        uint64 GetGUID(int32 id) const
        {
            switch (id)
            {
                case PLAYER_GUID:
                    return m_playerGUID;
                case NPC_DARIUS_CROWLEY:
                    return m_dariusGUID;
            }
            return 0;
        }

        void SetGUID(uint64 guid, int32 id) override
        {
            switch (id)
            {
                case PLAYER_GUID:
                {
                    m_playerGUID = guid;
                    break;
                }
                case NPC_DARIUS_CROWLEY:
                {
                    m_dariusGUID = guid;
                    break;
                }
            }
        }

        void DoAction(int32 param) override
        {
            if (param == 2)
                m_events.ScheduleEvent(EVENT_MOVE_TO_START, 500);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (m_movePart == 1)
                if (type == POINT_MOTION_TYPE)
                    if (id == MOVE_START_POSITION)
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* crowley = sObjectAccessor->GetCreature(*me, m_dariusGUID))
                            {
                                player->SetClientControl(me, 0);
                                crowley->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                me->GetMotionMaster()->MoveJump(-1714.02f, 1666.37f, 20.57f, 25.0f, 15.0f);
                                m_movePart = 2;
                                return;
                            }

            if (m_movePart == 2)
            {
                me->GetMotionMaster()->MovePath(352311, false);
                m_movePart = 3;
                return;
            }

            if (m_movePart == 3)
                if (type == WAYPOINT_MOTION_TYPE)
                    switch (id)
                {
                    case 2:
                        if (Creature* crowley = sObjectAccessor->GetCreature(*me, m_dariusGUID))
                        {
                            crowley->AI()->Talk(SAY_CROWLEY_HORSE_1);
                            m_events.ScheduleEvent(EVENT_CROWLEY_SAY, 6000);
                            m_events.ScheduleEvent(EVENT_CAST_TORCH, 1000);
                        }
                        break;
                    case 16:
                        m_events.ScheduleEvent(EVENT_JUMP_ON_BRIDGE, 25);
                        return;
                }

            if (m_movePart == 4)
            {
                m_movePart = 5;
                me->GetMotionMaster()->MovePath(352312, false);
                return;
            }

            if (m_movePart == 5)
                if (type == WAYPOINT_MOTION_TYPE)
                    switch (id)
                {
                    case 12:
                        m_events.Reset();
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->getThreatManager().resetAllAggro();
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                        {
                            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            player->getThreatManager().resetAllAggro();
                        }
                        break;
                    case 14:
                        if (Creature* crowley = sObjectAccessor->GetCreature(*me, m_dariusGUID))
                        {
                            crowley->ExitVehicle();
                            crowley->DespawnOrUnsummon(100);
                        }
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                        {
                            player->SetClientControl(me, 1);
                            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                            player->ExitVehicle();
                            me->DespawnOrUnsummon(100);
                            if (player->GetQuestStatus(QUEST_SACRIFICES) != QUEST_STATUS_COMPLETE)
                            {
                                player->FailQuest(QUEST_SACRIFICES);
                                player->NearTeleportTo(-1739.2f, 1657.9f, 20.48f, 0.5225f);
                            }
                        }
                        break;
                }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                if (player->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                {
                    player->FailQuest(QUEST_SACRIFICES);
                    player->NearTeleportTo(-1739.2f, 1657.9f, 20.48f, 0.5225f);
                }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CROWLEY_SAY:
                    {
                        if (Creature* crowley = sObjectAccessor->GetCreature(*me, m_dariusGUID))
                            crowley->AI()->Talk(SAY_CROWLEY_HORSE_2);
                        m_events.ScheduleEvent(EVENT_CROWLEY_SAY, urand(5000, 7000));
                        break;
                    }
                    case EVENT_MOVE_TO_START:
                    {
                        if (CharmInfo* info = me->InitCharmInfo())
                        {
                            info->InitEmptyActionBar(false);
                            info->SetActionBar(0, SPELL_THROW_TORCH, ACT_PASSIVE);
                        }

                        m_movePart = 1;
                        me->GetMotionMaster()->MovePoint(MOVE_START_POSITION, -1735.01f, 1653.01f, 20.49f);
                        break;
                    }
                    case EVENT_JUMP_ON_BRIDGE:
                    {
                        m_movePart = 4;
                        me->GetMotionMaster()->MoveJump(-1571.23f, 1710.034f, 20.485f, 25.0f, 15.0f);
                        break;
                    }
                    case EVENT_CAST_TORCH:
                    {
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* worgen = me->FindRandomCreatureInRange(NPC_BLOODFANG_STALKER, 25.0f, true))
                                player->CastSpell(worgen, SPELL_THROW_TORCH, true);

                        m_events.ScheduleEvent(EVENT_CAST_TORCH, urand(3000, 4000));
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_crowley_horse_35231AI(creature);
    }
};

// 35230
class npc_lord_darius_crowley_35230 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35230() : CreatureScript("npc_lord_darius_crowley_35230") {}

    enum eNpc
    {
        QUEST_SACRIFICES = 14212,
        NPC_LORD_DARIUS_CROWLEY = 35552,
        NPC_DARIUS_CROWLEY = 35230,
        NPC_CROWLEY_HORSE = 35231,
        SPELL_CROWLEY_FORCE_SUMMON = 67004,
        SPELL_RIDE_HORSE = 43671,
        PLAYER_GUID = 99999,
        SAY_CROWLEY_HORSE_1 = 0,
        SAY_CROWLEY_HORSE_2 = 1,
        MOVE_START_POSITION = 102,
        EVENT_CROWLEY_SAY = 101,
        EVENT_MOVE_TO_START = 102,
    };

    struct npc_lord_darius_crowley_35230AI : public ScriptedAI
    {
        npc_lord_darius_crowley_35230AI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_events;
        uint64 m_horseGUID;
        uint64 m_playerGUID;
        uint8  m_movePart;

        void AttackStart(Unit* /*who*/) override {}
        void EnterCombat(Unit* /*who*/) override {}
        void EnterEvadeMode() override {}

        void Reset() override
        {
            m_horseGUID = NULL;
            m_playerGUID = NULL;
            m_movePart = 0;
        }

        void IsSummonedBy(Unit* summoner) override
        {
            if (Creature* horse = summoner->ToCreature())
                if (horse->GetEntry() == NPC_CROWLEY_HORSE)
                {
                    m_horseGUID = horse->GetGUID();
                    m_playerGUID = horse->GetAI()->GetGUID(PLAYER_GUID);
                    m_events.ScheduleEvent(EVENT_MOVE_TO_START, 1000);
                }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CROWLEY_SAY:
                    {
                        Talk(SAY_CROWLEY_HORSE_2);
                        m_events.ScheduleEvent(EVENT_CROWLEY_SAY, urand(5000, 7000));
                        break;
                    }
                    case EVENT_MOVE_TO_START:
                    {
                        m_movePart = 1;
                        if (Player* player = sObjectAccessor->GetPlayer(*me, m_playerGUID))
                            if (Creature* horse = sObjectAccessor->GetCreature(*me, m_horseGUID))
                            {
                                player->SetClientControl(horse, /* bool allowMove */ false);  
                                // horse->SetAI(new npc_crowley_horse_35231::npc_crowley_horse_35231AI(horse));
                                // horse->setActive(true);      // move no ... torch yes..
                                horse->EnableAI();           // move yes... torch no..
                                // horse->IsAIEnabled = true;   // move no ... torch yes..
                                horse->GetAI()->SetGUID(m_playerGUID, PLAYER_GUID);
                                horse->GetAI()->SetGUID(me->GetGUID(), NPC_DARIUS_CROWLEY);
                                horse->GetAI()->DoAction(2);
                            }
                        break;
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_lord_darius_crowley_35230AI(creature);
    }
};

// 67063
class spell_throw_torch_67063 : public SpellScriptLoader
{
public:
    spell_throw_torch_67063() : SpellScriptLoader("spell_throw_torch_67063") { }

    enum eNpc
    {
        NPC_BLOODFANG_STALKER = 35229,
    };

    class FriendlyCheck
    {
    public:
        FriendlyCheck(Unit* caster) : _caster(caster) { }

        bool operator()(WorldObject* object) const
        {
            if (Unit* unit = object->ToUnit())
                return !unit->IsHostileTo(_caster);
            return true;
        }

    private:
        Unit* _caster;
    };

    class IsNotStalker
    {
    public:
        IsNotStalker(){}

        bool operator()(WorldObject* object) const
        {
            if (Unit* unit = object->ToUnit())
                return unit->GetEntry() != NPC_BLOODFANG_STALKER;
            return true;
        }
    };

    class spell_throw_torch_67063_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_throw_torch_67063_SpellScript);

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            targets.remove_if(IsNotStalker()); // (FriendlyCheck(GetCaster()));
            uint8 rol = urand(3, 8);
            while (targets.size() > rol)
                targets.erase(targets.begin());
        }

        void Register() override
        {
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_throw_torch_67063_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_throw_torch_67063_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_throw_torch_67063_SpellScript::FilterTargets, EFFECT_2, TARGET_UNIT_DEST_AREA_ENTRY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_throw_torch_67063_SpellScript();
    }
};

// 35229
class npc_bloodfang_stalker_35229 : public CreatureScript
{
public:
    npc_bloodfang_stalker_35229() : CreatureScript("npc_bloodfang_stalker_35229") {}

    enum eNpc
    {
        QUEST_SACRIFICES = 14212,
        NPC_NORTHGATE_REBEL_1 = 36057,
        NPC_CROWLEY_HORSE = 35231,
        NPC_BLOODFANG_STALKER_CREDIT = 35582,
        NPC_TOBIAS_MISTMANTLE = 35618,
        SPELL_THROW_TORCH = 67063,
        SPELL_ENRAGE = 8599,
        COOLDOWN_ENRAGE = 30000,
        SOUND_WORGEN_ATTACK = 558,
        EVENT_ENRAGE_COOLDOWN = 101,
        EVENT_CHECK_CROWLEY,

    };

    struct npc_bloodfang_stalker_35229AI : public ScriptedAI
    {
        npc_bloodfang_stalker_35229AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        bool m_enrage;

        void Reset() override
        {
            if (me->HasAura(SPELL_THROW_TORCH))
                me->RemoveAura(SPELL_THROW_TORCH);
            m_enrage = false;
            m_events.Reset();    
            m_events.ScheduleEvent(EVENT_CHECK_CROWLEY, 1000);
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (!m_enrage && me->GetHealthPct() < 90.0f)
            {
                me->CastSpell(me, SPELL_ENRAGE);
                Talk(1);
                m_enrage = true;
                m_events.ScheduleEvent(EVENT_ENRAGE_COOLDOWN, urand(121000, 150000));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENRAGE_COOLDOWN:
                    {
                        m_enrage = false;
                        break;
                    }
                    case EVENT_CHECK_CROWLEY:
                    {
                        if (me->FindNearestCreature(NPC_TOBIAS_MISTMANTLE, 10.0f))
                            me->DespawnOrUnsummon(25);
                        else if (me->HasAura(SPELL_THROW_TORCH))
                            me->DespawnOrUnsummon(5000);
                        else
                            m_events.ScheduleEvent(EVENT_CHECK_CROWLEY, 1000);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;

            if (me->GetVictim()->GetTypeId() == TYPEID_PLAYER)
                Miss = false;
            else if (me->GetVictim()->IsPet())
                Miss = false;
            else if (me->GetVictim()->GetEntry() == NPC_NORTHGATE_REBEL_1)
                if (me->GetVictim()->GetHealthPct() < 90)
                    Miss = true;

            if (Miss && tAnimate <= diff)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_UNARMED);
                me->PlayDistanceSound(SOUND_WORGEN_ATTACK);
                tAnimate = 2000;
            }
            else
                tAnimate -= diff;

            if (!Miss)
                DoMeleeAttackIfReady();
        }

        void SpellHit(Unit* caster, const SpellInfo* spell) override
        {
            if (spell->Id == SPELL_THROW_TORCH)
                if (Player* player = caster->ToPlayer())
                    if (player->GetQuestStatus(QUEST_SACRIFICES) == QUEST_STATUS_INCOMPLETE)
                    {
                        caster->ToPlayer()->KilledMonsterCredit(NPC_BLOODFANG_STALKER_CREDIT);
                        me->DespawnOrUnsummon(5000);
                    }
        }

    private:
        uint32 tAnimate;
        bool Miss;
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_bloodfang_stalker_35229AI(creature);
    }
};

// 44468
class npc_sister_almyra_44468 : public CreatureScript
{
public:
    npc_sister_almyra_44468() : CreatureScript("npc_sister_almyra_44468") { }

    enum eNPC
    {
        SPELL_RENEW = 139,
        EVENT_CAST_RENEW = 101,
    };

    struct npc_sister_almyra_44468AI : public ScriptedAI
    {
        npc_sister_almyra_44468AI(Creature* creature) : ScriptedAI(creature) { }

        EventMap m_events;

        void Reset() override
        {
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CAST_RENEW, 1000);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CAST_RENEW:
                    {
                        if (Player* player = me->FindRandomPlayerInRange(40.0f, true))
                            if (!player->HasAura(SPELL_RENEW))
                                me->CastSpell(player, SPELL_RENEW);

                        m_events.ScheduleEvent(EVENT_CAST_RENEW, 5000);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_sister_almyra_44468AI(creature);
    }
};

// 35317
class npc_rebel_cannon_35317 : public CreatureScript
{
public:
    npc_rebel_cannon_35317() : CreatureScript("npc_rebel_cannon_35317") { }

    enum eNPC
    {
        NPC_BLOODFANG_STALKER = 35229,
        EVENT_CHECK_STALKER = 102,
    };

    struct npc_rebel_cannon_35317AI : public ScriptedAI
    {
        npc_rebel_cannon_35317AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;

        void Reset() override
        {
            m_events.ScheduleEvent(EVENT_CHECK_STALKER, 1000);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_STALKER:
                    {
                        std::list<Creature*> cList = me->FindNearestCreatures(NPC_BLOODFANG_STALKER, 4.0f);
                        for (std::list<Creature*>::const_iterator itr = cList.begin(); itr != cList.end(); ++itr)
                        {
                            if (Creature* stalker = *itr)
                                stalker->DespawnOrUnsummon(25);
                        }
                        m_events.ScheduleEvent(EVENT_CHECK_STALKER, 1000);
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_rebel_cannon_35317AI(creature);
    }
};

// 36057 Northgate Rebel
class npc_northgate_rebel_36057 : public CreatureScript
{
public:
    npc_northgate_rebel_36057() : CreatureScript("npc_northgate_rebel_36057") { }

    enum eNPC
    {
        NPC_BLOODFANG_STALKER = 35229,
        EVENT_CHECK_SHOWFIGHT = 101,
        MOVE_TO_HOMEPOSITION,
    };

    struct npc_northgate_rebel_36057AI : public ScriptedAI
    {
        npc_northgate_rebel_36057AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        float    m_minHealthPct;
        bool     m_isShowFight;
        uint32    m_minDamage;
        uint32    m_maxDamage;
        std::list<uint32> m_targetList;

        void Initialize()
        {
            m_targetList.clear();
            m_targetList.push_back(NPC_BLOODFANG_STALKER);
            m_minHealthPct = frand(30.0f, 85.0f);
            m_minDamage = 1;
            m_maxDamage = 1;
        }

        void Reset() override
        {
            m_events.Reset();
            m_isShowFight = true;
            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 1000);
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE && pointId == MOVE_TO_HOMEPOSITION)
                m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 25);
        }

        uint32 FindTargetEntry(uint32 entry)
        {
            if (m_targetList.empty())
                return 0;

            for (std::list<uint32>::const_iterator itr = m_targetList.begin(); itr != m_targetList.end(); ++itr)
                if ((*itr) == entry)
                    return entry;

            return 0;
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (m_isShowFight && who->ToCreature())
                if (FindTargetEntry(who->GetEntry()))
                {
                    if (damage > m_maxDamage)
                        damage = m_maxDamage;

                    if (me->GetHealthPct() < m_minHealthPct)
                        damage = 0;
                }
                    
        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType damageType) override
        {
            if (m_isShowFight && victim->ToCreature())
                if (FindTargetEntry(victim->GetEntry()))
                {
                    if (damage > m_maxDamage)
                        damage = m_maxDamage;

                    if (victim->GetHealthPct() < m_minHealthPct)
                        damage = m_minDamage;
                }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                case EVENT_CHECK_SHOWFIGHT:
                {
                    if (me->IsInCombat() || me->IsDead())
                    {
                        m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                        return;
                    }

                    if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 10.0f)
                    {
                        me->GetMotionMaster()->MovePoint(MOVE_TO_HOMEPOSITION, me->GetHomePosition());
                        return;
                    }

                    if (Creature* worgen = me->FindNearestCreature(m_targetList, 5.0f))
                    {
                        me->Attack(worgen, true);
                        worgen->Attack(me, true);
                        m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                        return;
                    }

                    m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                    break;
                }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_northgate_rebel_36057AI(creature);
    }
};

/* Phase 1024 - START */

// 35566
class npc_lord_darius_crowley_35566 : public CreatureScript
{
public:
    npc_lord_darius_crowley_35566() : CreatureScript("npc_lord_darius_crowley_35566") {}

    enum eNpc
    {
        QUEST_LAST_STAND = 14222,
        NPC_FRENZIED_STALKER = 35627,
        PLAYER_GUID = 99999,
        SPELL_INFECTED_BITE = 72872,            // phase 8
        SPELL_HIDEOUS_BITE_WOUND = 76642,       // phase 1024

        // SPELL_TWO_FORMS = 68996,             // Server-side script 
        // SPELL_IN_STOCKS = 69196,             // Server-side script 
        // SPELL_LAST_STAND_COMPLETE = 72788,   // is called triggered and trigger more spell=95759/learn-darkflight-internal // spell=72792/learn-worgen-racials-1
        // SPELL_TRANSFORMING = 72794,          // trigger spell=72788/last-stand-complete // spell=93477/play-movie
        SPELL_FADE_OF_BLACK = 94053,

        SPELL_ALTERED_FORM = 94293,             // Apply (Aura #352)
        SPELL_FORCE_WORGEN_ALTERED_FORM = 98274,// Veränderte Worgenform erzwingen
        SPELL_ALTERED_FORM2 = 97709,            // Alternative Gestalt (Volksfähigkeit)
        SPELL_CURSE_OF_THE_WORGEN = 68630,      // (SPELL_EFFECT_TELEPORT_UNITS), Trigger spell (68631 (Invisible, Stun)) Trigger spell (69123 (Summon 36331)) 
    };

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_LAST_STAND)
        {
            if (player->HasAura(SPELL_INFECTED_BITE))
                player->RemoveAura(SPELL_INFECTED_BITE);
            player->AddAura(SPELL_HIDEOUS_BITE_WOUND, player);
        }
        return true;
    }

    bool OnQuestReward(Player* player, Creature* creature, const Quest *_Quest, uint32) override
    {
        if (_Quest->GetQuestId() == QUEST_LAST_STAND)
        {
            WorldLocation loc;
            loc.m_mapId = 654;
            loc.m_positionX = -1818.4f;
            loc.m_positionY = 2294.25f;
            loc.m_positionZ = 42.2135f;
            loc.m_orientation = 3.14f;
            player->SetHomebind(loc, 4786);

            player->RemoveAura(SPELL_HIDEOUS_BITE_WOUND);
            player->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_DISABLE_TURN);
            player->CastSpell(player, SPELL_FADE_OF_BLACK, true);
            player->CastSpell(player, SPELL_ALTERED_FORM, true);
            player->CastSpell(player, SPELL_FORCE_WORGEN_ALTERED_FORM, true);
            player->CastSpell(player, SPELL_ALTERED_FORM2, true);
            player->CastSpell(player, SPELL_CURSE_OF_THE_WORGEN, true);
            player->TeleportTo(loc);
        }
        return true;
    }
};

// 35627
class npc_frenzied_stalker_35627 : public CreatureScript
{
public:
    npc_frenzied_stalker_35627() : CreatureScript("npc_frenzied_stalker_35627") {}

    enum eNpc
    {
        QUEST_LAST_STAND = 14222,
        NPC_FRENZIED_STALKER = 35627,
        SPELL_ENRAGE = 8599,
        EVENT_CHECK_SHOWFIGHT = 101,
        EVENT_ENRAGE_COOLDOWN,
        MOVE_TO_HOMEPOSITION,
    };

    struct npc_frenzied_stalker_35627AI : public ScriptedAI
    {
        npc_frenzied_stalker_35627AI(Creature* creature) : ScriptedAI(creature) {}

        EventMap m_events;
        bool m_enrage;

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            m_enrage = false;
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 1000);
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (!m_enrage && me->GetHealthPct() < 30.0f)
            {
                me->CastSpell(me, SPELL_ENRAGE);
                Talk(1);
                m_enrage = true;
                m_events.ScheduleEvent(EVENT_ENRAGE_COOLDOWN, urand(121000, 150000));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ENRAGE_COOLDOWN:
                    {
                        m_enrage = false;
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_frenzied_stalker_35627AI(creature);
    }
};

// 41015
class npc_northgate_rebel_41015 : public CreatureScript
{
public:
    npc_northgate_rebel_41015() : CreatureScript("npc_northgate_rebel_41015") { }

    enum eNPC
    {
        QUEST_LAST_STAND = 14222,
        NPC_FRENZIED_STALKER = 35627,
        EVENT_CHECK_SHOWFIGHT = 101,
        MOVE_TO_HOMEPOSITION,
    };

    struct npc_northgate_rebel_41015AI : public ScriptedAI
    {
        npc_northgate_rebel_41015AI(Creature* creature) : ScriptedAI(creature) { Initialize(); }

        EventMap m_events;
        float    m_minHealthPct;
        bool     m_isShowFight;
        uint32    m_minDamage;
        uint32    m_maxDamage;
        std::list<uint32> m_targetList;

        void Initialize()
        {
            m_targetList.clear();
            m_targetList.push_back(NPC_FRENZIED_STALKER);
            m_minHealthPct = frand(55.0f, 85.0f);
            m_minDamage = 1;
            m_maxDamage = 1;
        }

        void Reset() override
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            m_isShowFight = true;
            m_events.Reset();
            m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 1000);
        }

        uint32 FindTargetEntry(uint32 entry)
        {
            if (m_targetList.empty())
                return 0;

            for (std::list<uint32>::const_iterator itr = m_targetList.begin(); itr != m_targetList.end(); ++itr)
                if ((*itr) == entry)
                    return entry;

            return 0;
        }

        void DamageTaken(Unit* who, uint32 &damage) override
        {
            if (m_isShowFight && who->ToCreature())
                if (FindTargetEntry(who->GetEntry()))
                {
                    if (damage > m_maxDamage)
                        damage = m_maxDamage;

                    if (me->GetHealthPct() < m_minHealthPct)
                        damage = 0;
                }

        }

        void DamageDealt(Unit* victim, uint32& damage, DamageEffectType damageType) override
        {
            if (m_isShowFight && victim->ToCreature())
                if (FindTargetEntry(victim->GetEntry()))
                {
                    if (damage > m_maxDamage)
                        damage = m_maxDamage;

                    if (victim->GetHealthPct() < m_minHealthPct)
                        damage = 0;
                }
        }

        void MovementInform(uint32 type, uint32 pointId) override
        {
            if (type == POINT_MOTION_TYPE && pointId == MOVE_TO_HOMEPOSITION)
                m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 25);
        }

        void UpdateAI(uint32 diff) override
        {
            m_events.Update(diff);

            while (uint32 eventId = m_events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_CHECK_SHOWFIGHT:
                    {
                        m_events.ScheduleEvent(EVENT_CHECK_SHOWFIGHT, 2500);
                        if (me->IsInCombat() || me->IsDead())
                            return;

                        if (me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) > 8.0f)
                        {
                            me->GetMotionMaster()->MovePoint(MOVE_TO_HOMEPOSITION, me->GetHomePosition());
                            return;
                        }

                        if (Creature* worgen = me->FindNearestCreature(m_targetList, 10.0f))
                        {
                            me->Attack(worgen, true);
                            worgen->Attack(me, true);
                            return;
                        }
                        break;
                    }
                }
            }

            if (!UpdateVictim())
                return;
            else
                DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_northgate_rebel_41015AI(creature);
    }
};

// next part in script zone_duskhaven

void AddSC_zone_gilneas_city()
{
    new npc_gilneas_city_guard_gate_34864();
    new npc_prince_liam_greymane_34850();
    new npc_rampaging_worgen_35660();
    new npc_gilneas_city_guard_34916();
    new npc_prince_liam_greymane_34913();
    new go_merchant_square_door_195327();
    new npc_rampaging_worgen_34884();
    new npc_frightened_citizen_34981();
    new npc_frightened_citizen_35836();
    new npc_sergeant_cleese_35839();
    new npc_gilnean_royal_guard_35232();
    new npc_mariam_spellwalker_35872();
    new npc_king_genn_greymane_35112();
    new npc_bloodfang_worgen_35118();
    new npc_tobias_mistmantle_35124();
    new npc_lord_darius_crowley_35077();
    new npc_worgen_runt_35188();
    new npc_worgen_alpha_35170();
    new npc_worgen_runt_35456();
    new npc_worgen_alpha_35167();
    new npc_lord_godfrey_35115();
    new npc_josiah_avery_35369();
    new npc_josiah_avery_trigger_50415();
    new npc_lorna_crowley_35378();
    new npc_bloodfang_lurker_35463();
    new npc_gilnean_mastiff_35631();
    new npc_lord_godfrey_35906();
    new npc_gilnean_city_guard_35504();
    new npc_king_genn_greymane_35550();
    new npc_king_greymanes_horse_35905();
    new npc_krennan_aranas_35907();
    new npc_commandeered_cannon_35914();
    new npc_bloodfang_stalker_35229();
    new npc_lord_darius_crowley_35552();
    new npc_sister_almyra_44468();
    new spell_csa_dummy_effect_56685();
    new npc_lord_darius_crowley_35230();
    new npc_crowley_horse_35231();
    new spell_throw_torch_67063();
    new npc_rebel_cannon_35317();
    new npc_northgate_rebel_36057();
    new npc_lord_darius_crowley_35566();
    new npc_northgate_rebel_41015();
    new npc_frenzied_stalker_35627();
};

