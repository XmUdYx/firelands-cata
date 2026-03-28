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

#include "PlayerbotsDatabase.h"
#include "MySQLPreparedStatement.h"

void PlayerbotsDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_PLAYERBOTSDATABASE_STATEMENTS);

    PrepareStatement(PLAYERBOTS_SEL_BOT_COUNT, "SELECT COUNT(*) FROM playerbots_bot", CONNECTION_SYNCH);
    PrepareStatement(PLAYERBOTS_SEL_BOT_BY_ACCOUNT, "SELECT id FROM playerbots_bot WHERE account_id = ? LIMIT 1", CONNECTION_SYNCH);
    PrepareStatement(PLAYERBOTS_INS_BOT,
                     "INSERT INTO playerbots_bot (account_id, character_guid, internal_name, status, flags) VALUES (?, ?, ?, 2, 0)",
                     CONNECTION_SYNCH);
    PrepareStatement(PLAYERBOTS_SEL_ACTIVE_BOTS,
                     "SELECT id, account_id, character_guid FROM playerbots_bot WHERE status = 2 AND character_guid IS NOT NULL",
                     CONNECTION_SYNCH);
    PrepareStatement(PLAYERBOTS_SEL_ACTIVE_BOT_COUNT,
                     "SELECT COUNT(*) FROM playerbots_bot WHERE status = 2 AND character_guid IS NOT NULL", CONNECTION_SYNCH);
}

PlayerbotsDatabaseConnection::PlayerbotsDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) { }

PlayerbotsDatabaseConnection::PlayerbotsDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo)
    : MySQLConnection(q, connInfo)
{
}

PlayerbotsDatabaseConnection::~PlayerbotsDatabaseConnection() = default;
