/*
 * This file is part of the FirelandsCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Affero General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _PLAYERBOTS_BOT_PERSISTENCE_REPOSITORY_H_
#define _PLAYERBOTS_BOT_PERSISTENCE_REPOSITORY_H_

#include "Define.h"

#include <cstdint>
#include <string>
#include <vector>

namespace Playerbots
{
struct PersistedActiveBot
{
    uint64 rowId = 0;
    uint32 accountId = 0;
    uint64 characterGuidLow = 0;
};

/// Data access for the dedicated `playerbots` MySQL database (see PlayerbotsDatabase pool).
class BotPersistenceRepository
{
public:
    BotPersistenceRepository() = default;

    /// Returns row count in playerbots_bot (0 if query fails, e.g. DB unavailable).
    uint64 CountBots() const;

    /// Rows eligible for headless login (status active + bound character guid).
    uint64 CountActivePersistedBots() const;

    bool HasBotForAccount(uint32 accountId) const;
    void InsertActiveBot(uint32 accountId, uint64 characterGuidLow, std::string const& internalName) const;

    void LoadActiveBots(std::vector<PersistedActiveBot>& out) const;
};
} // namespace Playerbots

#endif
