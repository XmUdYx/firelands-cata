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

#include "BotPersistenceRepository.h"
#include "DatabaseEnv.h"
#include "Define.h"

uint64 Playerbots::BotPersistenceRepository::CountBots() const
{
    QueryResult result = PlayerbotsDatabase.PQuery("SELECT COUNT(*) FROM playerbots_bot");
    if (!result)
        return 0;

    return result->Fetch()[0].GetUInt64();
}

uint64 Playerbots::BotPersistenceRepository::CountActivePersistedBots() const
{
    QueryResult result = PlayerbotsDatabase.PQuery(
        "SELECT COUNT(*) FROM playerbots_bot WHERE status = 2 AND character_guid IS NOT NULL");
    if (!result)
        return 0;

    return result->Fetch()[0].GetUInt64();
}

bool Playerbots::BotPersistenceRepository::HasBotForAccount(uint32 accountId) const
{
    return bool(PlayerbotsDatabase.PQuery("SELECT id FROM playerbots_bot WHERE account_id = %u LIMIT 1", accountId));
}

void Playerbots::BotPersistenceRepository::InsertActiveBot(uint32 accountId, uint64 characterGuidLow,
                                                           std::string const& internalName) const
{
    std::string escaped = internalName;
    PlayerbotsDatabase.EscapeString(escaped);
    PlayerbotsDatabase.DirectPExecute(
        "INSERT INTO playerbots_bot (account_id, character_guid, internal_name, status, flags) VALUES (%u, " UI64FMTD
        ", '%s', 2, 0)",
        accountId, characterGuidLow, escaped.c_str());
}

void Playerbots::BotPersistenceRepository::LoadActiveBots(std::vector<PersistedActiveBot>& out) const
{
    out.clear();
    QueryResult result = PlayerbotsDatabase.PQuery(
        "SELECT id, account_id, character_guid FROM playerbots_bot WHERE status = 2 AND character_guid IS NOT NULL");
    if (!result)
        return;

    do
    {
        Field* f = result->Fetch();
        PersistedActiveBot row;
        row.rowId = f[0].GetUInt64();
        row.accountId = f[1].GetUInt32();
        row.characterGuidLow = f[2].GetUInt64();
        out.push_back(row);
    } while (result->NextRow());
}
