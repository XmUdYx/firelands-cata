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

#ifndef _PLAYERBOTS_BOT_IDLE_MOVEMENT_SERVICE_H_
#define _PLAYERBOTS_BOT_IDLE_MOVEMENT_SERVICE_H_

#include "Define.h"
#include "ObjectGuid.h"
#include "Position.h"

#include <unordered_map>

class Player;

namespace Playerbots
{
/// Throttled idle / random nearby wander for managed headless bots (Phase 2 — movement only).
class BotIdleMovementService
{
public:
    static BotIdleMovementService& Instance();

    void OnPlayerUpdate(Player* player, uint32 diff);
    void OnPlayerLogout(Player* player);

private:
    BotIdleMovementService() = default;

    struct BotIdleState
    {
        uint32 accumMs = 0;
        uint32 nextThinkMs = 0;
        uint32 postMoveRelaxUntilMs = 0;
        bool hadActiveSpline = false;
        bool anchorInitialized = false;
        Position anchor;
        uint32 mapId = 0;
    };

    BotIdleState& GetOrCreateState(ObjectGuid const& guid);
    void RemoveState(ObjectGuid const& guid);

    bool CanConsiderIdle(Player const* player) const;
    bool TryRandomRelocate(Player* player, BotIdleState& st);
    void TryFaceOnly(Player* player, BotIdleState& st);
    void TryDistract(Player* player, BotIdleState& st);

    std::unordered_map<ObjectGuid, BotIdleState> _states;
};
} // namespace Playerbots

#endif
