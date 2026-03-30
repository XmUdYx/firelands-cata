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

#ifndef _PLAYERBOTS_BOT_TARGET_SELECTION_SERVICE_H_
#define _PLAYERBOTS_BOT_TARGET_SELECTION_SERVICE_H_

#include "Define.h"
#include "ObjectGuid.h"

#include <unordered_map>

class Creature;
class Player;

namespace Playerbots
{
/// Nearby hostile creature discovery + simple filters; exposes selection via ObjectGuid and Player::SetSelection.
class BotTargetSelectionService
{
public:
    static BotTargetSelectionService& Instance();

    void OnPlayerUpdate(Player* player, uint32 diff);
    void OnPlayerLogout(Player* player);

    /// Last chosen grind target for this bot (may be empty).
    ObjectGuid GetPrimaryTargetGuid(Player const* player) const;

    /// Resolves the creature in the player's map, or nullptr if missing / wrong type.
    Creature* GetPrimaryTargetCreature(Player* player) const;

    /// Clears stored primary target and server selection (after kill / disengage / invalid).
    void ClearPrimaryTarget(Player* player);

private:
    BotTargetSelectionService() = default;

    struct BotTargetState
    {
        ObjectGuid primaryTarget;
        uint32 accumMs = 0;
    };

    void RemoveState(ObjectGuid const& playerGuid);
    void ApplySelection(Player* player, ObjectGuid const& targetGuid);

    /// Clears selection and aborts when false (dead, loading, BG, movement edge cases).
    bool CanHoldTargetContext(Player const* player) const;
    /// When true, skip rescan but keep stored target (e.g. already in combat).
    bool ShouldFreezeRescan(Player const* player) const;
    Creature* FindNearestFilteredCreature(Player* player) const;

    std::unordered_map<ObjectGuid, BotTargetState> _states;
};
} // namespace Playerbots

#endif
