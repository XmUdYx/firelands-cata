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

#include "BotLoginCoordinator.h"
#include "BotLoginAccountRow.h"
#include "BotPersistenceRepository.h"
#include "PlayerbotsConfig.h"

#include "CharacterCache.h"
#include "Common.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "World.h"
#include "WorldSession.h"

#include <vector>

namespace Playerbots
{
BotLoginCoordinator& BotLoginCoordinator::Instance()
{
    static BotLoginCoordinator inst;
    return inst;
}

bool BotLoginCoordinator::LoginBot(uint32 accountId, uint64 characterGuidLow, std::string& err)
{
    err.clear();

    if (!Config::IsEnabled())
    {
        err = "Playerbots.Enable is off";
        return false;
    }

    ObjectGuid const charGuid = ObjectGuid::Create<HighGuid::Player>(ObjectGuid::LowType(characterGuidLow));

    if (ObjectAccessor::FindPlayer(charGuid))
    {
        err = "character already in world";
        return false;
    }

    if (CharacterCacheEntry const* info = sCharacterCache->GetCharacterCacheByGuid(charGuid))
    {
        if (info->AccountId != accountId)
        {
            err = "character cache account mismatch";
            return false;
        }
    }

    if (WorldSession* existing = sWorld->FindSession(accountId))
    {
        if (existing->GetPlayer() || existing->PlayerLoading())
        {
            err = "account already has an active or loading session";
            return false;
        }
    }

    std::string username;
    AccountTypes sec = SEC_PLAYER;
    uint8 expansion = 0;
    if (!LoadLoginAccountRowForWorldSession(accountId, username, sec, expansion, err))
        return false;

    WorldSession* session =
        new WorldSession(accountId, std::move(username), nullptr, sec, expansion, 0, LOCALE_enUS, 0, false);
    session->SetHeadlessBotSession(true);
    session->LoadPermissions();
    session->QueueServerSideCharacterLogin(charGuid);
    sWorld->AddSession(session);

    LOG_INFO("playerbots", "Queued headless world login: account %u character %s", accountId, charGuid.ToString().c_str());
    return true;
}

LoginAllPersistedResult BotLoginCoordinator::LoginAllPersistedBotsWithResult()
{
    LoginAllPersistedResult r;
    if (!Config::IsEnabled())
    {
        r.lastError = "Playerbots.Enable is off";
        return r;
    }

    BotPersistenceRepository repo;
    std::vector<PersistedActiveBot> rows;
    repo.LoadActiveBots(rows);
    r.totalRows = uint32(rows.size());

    for (PersistedActiveBot const& row : rows)
    {
        std::string err;
        if (LoginBot(row.accountId, row.characterGuidLow, err))
            ++r.queuedOk;
        else
        {
            ++r.failed;
            r.lastError = err;
            LOG_ERROR("playerbots", "LoginBot failed (account %u row " UI64FMTD "): %s", row.accountId, row.rowId, err.c_str());
        }
    }

    LOG_INFO("playerbots", "LoginAllPersistedBots finished: %u ok, %u failed (%u rows)", r.queuedOk, r.failed, r.totalRows);
    return r;
}

void BotLoginCoordinator::LoginAllPersistedBots()
{
    (void)LoginAllPersistedBotsWithResult();
}

void BotLoginCoordinator::LogoutAllManagedBots()
{
    (void)LogoutAllManagedBotsWithCount();
}

uint32 BotLoginCoordinator::LogoutAllManagedBotsWithCount()
{
    uint32 n = 0;
    std::unordered_set<uint32> const copy = _managedAccounts;
    for (uint32 accountId : copy)
    {
        if (WorldSession* s = sWorld->FindSession(accountId))
        {
            if (Player* p = s->GetPlayer())
            {
                s->LogoutPlayer(true);
                ++n;
            }
        }
    }
    return n;
}

void BotLoginCoordinator::RegisterManagedBot(uint32 accountId)
{
    _managedAccounts.insert(accountId);
}

void BotLoginCoordinator::UnregisterManagedBot(uint32 accountId)
{
    _managedAccounts.erase(accountId);
}

bool BotLoginCoordinator::IsManagedBot(Player const* player) const
{
    if (!player || !player->GetSession())
        return false;
    if (!player->GetSession()->IsHeadlessBotSession())
        return false;
    return _managedAccounts.count(player->GetSession()->GetAccountId()) != 0;
}
} // namespace Playerbots
