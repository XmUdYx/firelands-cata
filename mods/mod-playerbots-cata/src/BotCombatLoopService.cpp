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

#include "BotCombatLoopService.h"

#include "BotLoginCoordinator.h"
#include "BotTargetSelectionService.h"
#include "PlayerbotsConfig.h"

#include "Creature.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Unit.h"
#include "WorldSession.h"

namespace Playerbots
{
BotCombatLoopService& BotCombatLoopService::Instance()
{
    static BotCombatLoopService inst;
    return inst;
}

void BotCombatLoopService::RemoveState(ObjectGuid const& playerGuid)
{
    _states.erase(playerGuid);
}

bool BotCombatLoopService::CanRunCombat(Player const* player)
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
    if (player->IsCharmed())
        return false;
    return true;
}

bool BotCombatLoopService::IsVictimAcceptableForContinue(Player* player, Creature* victim)
{
    if (!victim || !victim->IsInWorld() || !victim->IsAlive())
        return false;
    if (!player->IsValidAttackTarget(victim))
        return false;
    if (victim->IsEvadingAttacks())
        return false;

    float const maxDist = Config::GetCombatMaxChaseYards();
    if (!player->IsWithinDistInMap(victim, maxDist))
        return false;

    if (!player->CanSeeOrDetect(victim))
        return false;

    return true;
}

void BotCombatLoopService::FullDisengage(Player* player)
{
    player->CombatStop(false);
    player->StopMoving();
    player->GetMotionMaster()->Clear();
}

void BotCombatLoopService::EngageMeleeChase(Player* player, Unit* victim)
{
    if (!victim || !player->Attack(victim, true))
        return;

    if (player->HasUnitState(UNIT_STATE_DISTRACTED))
    {
        player->ClearUnitState(UNIT_STATE_DISTRACTED);
        player->GetMotionMaster()->Clear();
    }

    player->GetMotionMaster()->MoveChase(victim);
}

void BotCombatLoopService::OnPlayerUpdate(Player* player, uint32 diff)
{
    if (!player)
        return;
    if (!Config::IsEnabled() || !Config::IsCombatLoopEnabled())
        return;
    if (!player->GetSession() || !player->GetSession()->IsHeadlessBotSession())
        return;
    if (!BotLoginCoordinator::Instance().IsManagedBot(player))
        return;

    BotCombatState& st = _states[player->GetGUID()];
    st.tickAccumMs += diff;

    uint32 const interval = Config::GetCombatUpdateIntervalMs();
    if (st.tickAccumMs < interval)
        return;
    uint32 const elapsed = st.tickAccumMs;
    st.tickAccumMs = 0;

    if (!CanRunCombat(player))
    {
        FullDisengage(player);
        BotTargetSelectionService::Instance().ClearPrimaryTarget(player);
        st.losBreakAccumMs = 0;
        return;
    }

    if (player->IsMounted())
    {
        FullDisengage(player);
        BotTargetSelectionService::Instance().ClearPrimaryTarget(player);
        st.losBreakAccumMs = 0;
        return;
    }

    ObjectGuid const primary = BotTargetSelectionService::Instance().GetPrimaryTargetGuid(player);

    if (primary.IsEmpty())
    {
        if (player->GetVictim() || player->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
            FullDisengage(player);
        st.losBreakAccumMs = 0;
        return;
    }

    Creature* victim = BotTargetSelectionService::Instance().GetPrimaryTargetCreature(player);
    if (!victim || !IsVictimAcceptableForContinue(player, victim))
    {
        FullDisengage(player);
        BotTargetSelectionService::Instance().ClearPrimaryTarget(player);
        st.losBreakAccumMs = 0;
        return;
    }

    bool const meleeOk = player->IsWithinMeleeRange(victim);
    bool const losOk = player->IsWithinLOSInMap(victim);
    if (!losOk && !meleeOk)
        st.losBreakAccumMs += elapsed;
    else
        st.losBreakAccumMs = 0;

    if (st.losBreakAccumMs >= Config::GetCombatLosBreakMs())
    {
        FullDisengage(player);
        BotTargetSelectionService::Instance().ClearPrimaryTarget(player);
        st.losBreakAccumMs = 0;
        return;
    }

    if (player->IsMovementPreventedByCasting())
        return;

    if (player->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING | UNIT_STATE_POSSESSED))
        return;

    if (player->GetVictim() != victim)
        EngageMeleeChase(player, victim);
    else if (!player->HasUnitState(UNIT_STATE_MELEE_ATTACKING))
        EngageMeleeChase(player, victim);
    else if (player->GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE &&
             !player->IsWithinMeleeRange(victim))
        EngageMeleeChase(player, victim);
}

void BotCombatLoopService::OnPlayerLogout(Player* player)
{
    if (!player)
        return;
    RemoveState(player->GetGUID());
}
} // namespace Playerbots
