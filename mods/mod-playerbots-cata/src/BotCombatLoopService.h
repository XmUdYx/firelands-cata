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

#ifndef _PLAYERBOTS_BOT_COMBAT_LOOP_SERVICE_H_
#define _PLAYERBOTS_BOT_COMBAT_LOOP_SERVICE_H_

#include "Define.h"
#include "ObjectGuid.h"

#include <unordered_map>

class Creature;
class Player;
class Unit;

namespace Playerbots
{
/// Minimal melee engagement: MoveChase + Attack; auto-attack via core Player update; disengage on invalid/kill.
class BotCombatLoopService
{
public:
    static BotCombatLoopService& Instance();

    void OnPlayerUpdate(Player* player, uint32 diff);
    void OnPlayerLogout(Player* player);

private:
    BotCombatLoopService() = default;

    struct BotCombatState
    {
        uint32 tickAccumMs = 0;
        uint32 losBreakAccumMs = 0;
    };

    void RemoveState(ObjectGuid const& playerGuid);
    static bool CanRunCombat(Player const* player);
    static bool IsVictimAcceptableForContinue(Player* player, Creature* victim);
    static void FullDisengage(Player* player);
    static void EngageMeleeChase(Player* player, Unit* victim);

    std::unordered_map<ObjectGuid, BotCombatState> _states;
};
} // namespace Playerbots

#endif
