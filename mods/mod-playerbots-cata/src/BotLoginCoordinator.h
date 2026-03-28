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

#ifndef _PLAYERBOTS_BOT_LOGIN_COORDINATOR_H_
#define _PLAYERBOTS_BOT_LOGIN_COORDINATOR_H_

#include "Define.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_set>

class Player;

namespace Playerbots
{
struct LoginAllPersistedResult
{
    uint32 totalRows = 0;
    uint32 queuedOk = 0;
    uint32 failed = 0;
    std::string lastError;
};

/// Owns headless WorldSession bootstrap for persisted bots (playerbots DB → world).
class BotLoginCoordinator
{
public:
    static BotLoginCoordinator& Instance();

    /// Queue AddSession + deferred character login (same DB path as a real client).
    bool LoginBot(uint32 accountId, uint64 characterGuidLow, std::string& err);

    /// Load `playerbots_bot` active rows and call LoginBot for each (skips failures, logs).
    void LoginAllPersistedBots();

    LoginAllPersistedResult LoginAllPersistedBotsWithResult();

    /// Log out every headless bot we currently track as online.
    void LogoutAllManagedBots();

    /// Same as LogoutAllManagedBots; returns how many players received LogoutPlayer(true).
    uint32 LogoutAllManagedBotsWithCount();

    void RegisterManagedBot(uint32 accountId);
    void UnregisterManagedBot(uint32 accountId);

    /// Accounts currently registered after a successful headless world login (until logout).
    size_t GetManagedAccountCount() const { return _managedAccounts.size(); }

    bool IsManagedBot(Player const* player) const;

private:
    BotLoginCoordinator() = default;

    std::unordered_set<uint32> _managedAccounts;
};
} // namespace Playerbots

#endif
