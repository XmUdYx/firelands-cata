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

#include "BotSessionBootstrap.h"
#include "BotLoginCoordinator.h"

namespace Playerbots
{
bool BotSessionBootstrap::LoginBot(uint32 accountId, uint64 characterGuidLow, std::string& err)
{
    return BotLoginCoordinator::Instance().LoginBot(accountId, characterGuidLow, err);
}

void BotSessionBootstrap::LoginAllPersistedBots()
{
    BotLoginCoordinator::Instance().LoginAllPersistedBots();
}

LoginAllPersistedResult BotSessionBootstrap::LoginAllPersistedBotsWithResult()
{
    return BotLoginCoordinator::Instance().LoginAllPersistedBotsWithResult();
}

void BotSessionBootstrap::LogoutAllManagedBots()
{
    BotLoginCoordinator::Instance().LogoutAllManagedBots();
}

uint32 BotSessionBootstrap::LogoutAllManagedBotsWithCount()
{
    return BotLoginCoordinator::Instance().LogoutAllManagedBotsWithCount();
}

size_t BotSessionBootstrap::GetManagedHeadlessBotCount()
{
    return BotLoginCoordinator::Instance().GetManagedAccountCount();
}
} // namespace Playerbots
