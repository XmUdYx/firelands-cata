/*
 * This file is part of the FirelandsCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Affero General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "BotTargetSelectionService.h"

#include "BotLoginCoordinator.h"
#include "PlayerbotsConfig.h"

#include "CellImpl.h"
#include "Creature.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SharedDefines.h"
#include "WorldSession.h"

namespace
{
bool BotCreaturePassesFilters(Player const* player, Creature const* c, float maxRange)
{
    if (!c || !c->IsInWorld())
        return false;

    Unit const* u = c;
    if (!u->IsAlive())
        return false;

    if (c->GetCreatureType() == CREATURE_TYPE_NON_COMBAT_PET)
        return false;

    if (c->IsTotem())
        return false;

    if (!u->isTargetableForAttack(false))
        return false;

    if (!player->IsWithinDistInMap(u, maxRange))
        return false;

    if (!player->CanSeeOrDetect(c))
        return false;

    if (!player->IsValidAttackTarget(u))
        return false;

    if (c->IsTrigger())
        return false;

    if (Playerbots::Config::IsTargetSelectionSkipCritters() && c->GetCreatureType() == CREATURE_TYPE_CRITTER)
        return false;

    if (c->IsEvadingAttacks())
        return false;

    CreatureTemplate const* ct = c->GetCreatureTemplate();
    if (!ct)
        return false;

    if (Playerbots::Config::IsTargetSelectionSkipBossFlag() &&
        (ct->type_flags & CREATURE_TYPE_FLAG_BOSS_MOB))
        return false;

    if (Playerbots::Config::IsTargetSelectionSkipDungeonBoss() && c->IsDungeonBoss())
        return false;

    if (Playerbots::Config::IsTargetSelectionSkipElites())
    {
        uint32 const r = ct->rank;
        if (r == CREATURE_ELITE_ELITE || r == CREATURE_ELITE_RAREELITE || r == CREATURE_ELITE_WORLDBOSS)
            return false;
    }

    uint8 const maxDelta = Playerbots::Config::GetTargetSelectionMaxLevelDelta();
    if (maxDelta > 0)
    {
        uint8 const pl = player->getLevel();
        uint8 const cl = c->getLevel();
        if (cl > pl && (cl - pl) > maxDelta)
            return false;
    }

    if (Playerbots::Config::IsTargetSelectionSkipCombatWithOthers() && c->IsInCombat() && !c->IsInCombatWith(player))
        return false;

    if (Playerbots::Config::IsTargetSelectionRequireLos() && !player->IsWithinLOSInMap(c))
        return false;

    return true;
}

class NearestPlayerbotCreatureTargetCheck
{
public:
    NearestPlayerbotCreatureTargetCheck(WorldObject const* obj, Player const* player, float range)
        : _obj(obj), _player(player), _range(range)
    {
    }

    bool operator()(Creature* c)
    {
        if (!BotCreaturePassesFilters(_player, c, _range))
            return false;

        _range = _obj->GetDistance(*c);
        return true;
    }

private:
    WorldObject const* _obj;
    Player const* _player;
    float _range;
};
} // namespace

namespace Playerbots
{
BotTargetSelectionService& BotTargetSelectionService::Instance()
{
    static BotTargetSelectionService inst;
    return inst;
}

void BotTargetSelectionService::RemoveState(ObjectGuid const& playerGuid)
{
    _states.erase(playerGuid);
}

ObjectGuid BotTargetSelectionService::GetPrimaryTargetGuid(Player const* player) const
{
    if (!player)
        return ObjectGuid::Empty;
    auto it = _states.find(player->GetGUID());
    if (it == _states.end())
        return ObjectGuid::Empty;
    return it->second.primaryTarget;
}

Creature* BotTargetSelectionService::GetPrimaryTargetCreature(Player* player) const
{
    if (!player)
        return nullptr;
    ObjectGuid g = GetPrimaryTargetGuid(player);
    if (g.IsEmpty())
        return nullptr;
    return ObjectAccessor::GetCreature(*player, g);
}

void BotTargetSelectionService::ApplySelection(Player* player, ObjectGuid const& targetGuid)
{
    BotTargetState& st = _states[player->GetGUID()];
    st.primaryTarget = targetGuid;
    player->SetSelection(targetGuid);
}

bool BotTargetSelectionService::CanHoldTargetContext(Player const* player) const
{
    if (!player->IsInWorld() || player->IsDuringRemoveFromWorld() || player->IsLoading())
        return false;
    if (!player->IsAlive())
        return false;
    if (player->InBattleground())
        return false;
    if (player->IsBeingTeleported() || player->IsInFlight())
        return false;
    if (player->GetDirectTransport())
        return false;
    return true;
}

bool BotTargetSelectionService::ShouldFreezeRescan(Player const* player) const
{
    return player->IsInCombat();
}

Creature* BotTargetSelectionService::FindNearestFilteredCreature(Player* player) const
{
    float range = Config::GetTargetSelectionSearchRadius();
    Creature* result = nullptr;
    NearestPlayerbotCreatureTargetCheck check(player, player, range);
    Firelands::CreatureLastSearcher<NearestPlayerbotCreatureTargetCheck> searcher(player, result, check);
    Cell::VisitAllObjects(player, searcher, range);
    return result;
}

void BotTargetSelectionService::OnPlayerUpdate(Player* player, uint32 diff)
{
    if (!player)
        return;
    if (!Config::IsEnabled() || !Config::IsTargetSelectionEnabled())
        return;
    if (!player->GetSession() || !player->GetSession()->IsHeadlessBotSession())
        return;
    if (!BotLoginCoordinator::Instance().IsManagedBot(player))
        return;

    BotTargetState& st = _states[player->GetGUID()];
    st.accumMs += diff;
    if (st.accumMs < Config::GetTargetSelectionUpdateIntervalMs())
        return;
    st.accumMs = 0;

    if (!CanHoldTargetContext(player))
    {
        if (!st.primaryTarget.IsEmpty())
        {
            st.primaryTarget = ObjectGuid::Empty;
            player->SetSelection(ObjectGuid::Empty);
        }
        return;
    }

    if (ShouldFreezeRescan(player))
        return;

    float const maxRange = Config::GetTargetSelectionSearchRadius();

    if (!st.primaryTarget.IsEmpty())
    {
        if (Creature* cur = ObjectAccessor::GetCreature(*player, st.primaryTarget))
        {
            if (BotCreaturePassesFilters(player, cur, maxRange))
                return;
        }
    }

    Creature* found = FindNearestFilteredCreature(player);
    ObjectGuid newGuid = found ? found->GetGUID() : ObjectGuid::Empty;
    if (newGuid != st.primaryTarget)
        ApplySelection(player, newGuid);
}

void BotTargetSelectionService::OnPlayerLogout(Player* player)
{
    if (!player)
        return;
    RemoveState(player->GetGUID());
}
} // namespace Playerbots
