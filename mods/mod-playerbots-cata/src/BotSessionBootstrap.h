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

#ifndef _PLAYERBOTS_BOT_SESSION_BOOTSTRAP_H_
#define _PLAYERBOTS_BOT_SESSION_BOOTSTRAP_H_

#include "Define.h"

#include "BotLoginCoordinator.h"

#include <cstdint>
#include <string>

namespace Playerbots
{
/// Thin facade over BotLoginCoordinator (headless WorldSession + core login pipeline).
class BotSessionBootstrap
{
public:
    static bool LoginBot(uint32 accountId, uint64 characterGuidLow, std::string& err);
    static void LoginAllPersistedBots();
    static LoginAllPersistedResult LoginAllPersistedBotsWithResult();
    static void LogoutAllManagedBots();
    static uint32 LogoutAllManagedBotsWithCount();
    static size_t GetManagedHeadlessBotCount();
};
} // namespace Playerbots

#endif
