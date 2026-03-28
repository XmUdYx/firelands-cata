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

#ifndef _PLAYERBOTSDATABASE_H
#define _PLAYERBOTSDATABASE_H

#include "MySQLConnection.h"

enum PlayerbotsDatabaseStatements : uint32
{
    PLAYERBOTS_SEL_BOT_COUNT,
    PLAYERBOTS_SEL_BOT_BY_ACCOUNT,
    PLAYERBOTS_INS_BOT,
    PLAYERBOTS_SEL_ACTIVE_BOTS,
    PLAYERBOTS_SEL_ACTIVE_BOT_COUNT,

    MAX_PLAYERBOTSDATABASE_STATEMENTS
};

class FC_DATABASE_API PlayerbotsDatabaseConnection : public MySQLConnection
{
public:
    typedef PlayerbotsDatabaseStatements Statements;

    PlayerbotsDatabaseConnection(MySQLConnectionInfo& connInfo);
    PlayerbotsDatabaseConnection(ProducerConsumerQueue<SQLOperation*>* q, MySQLConnectionInfo& connInfo);
    ~PlayerbotsDatabaseConnection();

    void DoPrepareStatements() override;
};

#endif
