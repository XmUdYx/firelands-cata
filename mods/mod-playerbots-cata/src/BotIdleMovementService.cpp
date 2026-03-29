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

#include "BotIdleMovementService.h"

#include "BotLoginCoordinator.h"
#include "PlayerbotsConfig.h"

#include "GameTime.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "GridDefines.h"
#include "Player.h"
#include "Random.h"
#include "Unit.h"
#include "WorldSession.h"

#include <cmath>

namespace
{
constexpr uint32 PLAYERBOTS_IDLE_MOVE_POINT_ID = 91001;

bool IsFinitePosition(Player const* player)
{
    float x, y, z;
    player->GetPosition(x, y, z);
    return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
}
} // namespace

namespace Playerbots
{
BotIdleMovementService& BotIdleMovementService::Instance()
{
    static BotIdleMovementService inst;
    return inst;
}

BotIdleMovementService::BotIdleState& BotIdleMovementService::GetOrCreateState(ObjectGuid const& guid)
{
    return _states[guid];
}

void BotIdleMovementService::RemoveState(ObjectGuid const& guid)
{
    _states.erase(guid);
}

bool BotIdleMovementService::CanConsiderIdle(Player const* player) const
{
    if (!player->IsInWorld() || player->IsDuringRemoveFromWorld() || player->IsLoading())
        return false;

    if (!player->IsAlive() || player->IsInCombat() || player->InBattleground())
        return false;

    if (player->IsBeingTeleported() || player->IsInFlight())
        return false;

    if (player->GetDirectTransport())
        return false;

    if (player->IsMounted() || player->IsSitState())
        return false;

    if (player->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING | UNIT_STATE_POSSESSED))
        return false;

    if (player->IsCharmed())
        return false;

    if (player->IsMovementPreventedByCasting())
        return false;

    if (!IsFinitePosition(player))
        return false;

    if (Map* map = player->GetMap())
    {
        float x, y, z;
        player->GetPosition(x, y, z);
        if (map->GetHeight(player->GetPhaseShift(), x, y, z) <= INVALID_HEIGHT)
            return false;
    }
    else
        return false;

    return true;
}

void BotIdleMovementService::TryFaceOnly(Player* player, BotIdleState& st)
{
    float const o = frand(0.0f, float(2 * M_PI));
    player->SetFacingTo(o);
    st.nextThinkMs = GameTime::GetGameTimeMS() + urand(Config::GetIdleMinActionDelayMs(), Config::GetIdleMaxActionDelayMs());
}

void BotIdleMovementService::TryDistract(Player* player, BotIdleState& st)
{
    if (player->GetMotionMaster()->GetMotionSlot(MOTION_SLOT_CONTROLLED))
    {
        TryFaceOnly(player, st);
        return;
    }

    uint32 const ms = urand(Config::GetIdleDistractMinMs(), Config::GetIdleDistractMaxMs());
    player->GetMotionMaster()->MoveDistract(ms);
    st.nextThinkMs = GameTime::GetGameTimeMS() + ms + urand(500, 1500);
}

bool BotIdleMovementService::TryRandomRelocate(Player* player, BotIdleState& st)
{
    if (!st.anchorInitialized || st.mapId != player->GetMapId())
    {
        st.anchor = player->GetPosition();
        st.mapId = player->GetMapId();
        st.anchorInitialized = true;
    }

    float const radius = Config::GetIdleWanderRadiusYards();
    float const minStep = Config::GetIdleMinStepYards();
    float const maxStep = Config::GetIdleMaxStepYards();

    for (uint32 attempt = 0; attempt < 6; ++attempt)
    {
        float const angle = frand(0.0f, float(2 * M_PI));
        float const dist = frand(minStep, maxStep);

        float x = st.anchor.GetPositionX() + dist * std::cos(angle);
        float y = st.anchor.GetPositionY() + dist * std::sin(angle);
        float z = st.anchor.GetPositionZ();

        Firelands::NormalizeMapCoord(x);
        Firelands::NormalizeMapCoord(y);
        player->UpdateAllowedPositionZ(x, y, z);

        if (player->GetMap()->GetHeight(player->GetPhaseShift(), x, y, z) <= INVALID_HEIGHT)
            continue;

        if (st.anchor.GetExactDist2d(x, y) > radius)
            continue;

        if (player->GetExactDist2d(x, y) < 0.5f)
            continue;

        player->GetMotionMaster()->MovePoint(PLAYERBOTS_IDLE_MOVE_POINT_ID, x, y, z, true, 0.0f);
        st.hadActiveSpline = true;
        st.nextThinkMs = GameTime::GetGameTimeMS();
        return true;
    }

    return false;
}

void BotIdleMovementService::OnPlayerUpdate(Player* player, uint32 diff)
{
    if (!player)
        return;
    if (!Config::IsEnabled() || !Config::IsIdleMovementEnabled())
        return;
    if (!player->GetSession() || !player->GetSession()->IsHeadlessBotSession())
        return;
    if (!BotLoginCoordinator::Instance().IsManagedBot(player))
        return;

    BotIdleState& st = GetOrCreateState(player->GetGUID());
    st.accumMs += diff;
    uint32 const tick = Config::GetIdleUpdateIntervalMs();
    if (st.accumMs < tick)
        return;
    st.accumMs = 0;

    bool const splineFinalized = player->movespline->Finalized();

    if (st.hadActiveSpline && splineFinalized)
    {
        st.hadActiveSpline = false;
        uint32 const nowDone = GameTime::GetGameTimeMS();
        st.postMoveRelaxUntilMs = nowDone + urand(Config::GetIdlePostMovePauseMinMs(), Config::GetIdlePostMovePauseMaxMs());
        st.nextThinkMs = nowDone;
        st.anchor = player->GetPosition();
        st.mapId = player->GetMapId();
    }

    uint32 const now = GameTime::GetGameTimeMS();

    if (!splineFinalized)
        return;

    if (now < st.postMoveRelaxUntilMs)
        return;

    if (player->HasUnitState(UNIT_STATE_DISTRACTED))
        return;

    if (!CanConsiderIdle(player))
        return;

    if (now < st.nextThinkMs)
        return;

    if (!st.anchorInitialized || st.mapId != player->GetMapId())
    {
        st.anchor = player->GetPosition();
        st.mapId = player->GetMapId();
        st.anchorInitialized = true;
    }

    uint32 const roll = urand(0, 99);
    if (roll < Config::GetIdleChanceFacePct())
        TryFaceOnly(player, st);
    else if (roll < Config::GetIdleChanceFacePct() + Config::GetIdleChanceDistractPct())
        TryDistract(player, st);
    else
    {
        if (!TryRandomRelocate(player, st))
        {
            TryFaceOnly(player, st);
        }
    }
}

void BotIdleMovementService::OnPlayerLogout(Player* player)
{
    if (!player)
        return;
    RemoveState(player->GetGUID());
}
} // namespace Playerbots
