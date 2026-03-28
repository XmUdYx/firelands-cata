/*
 * This file is part of the FirelandsCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#ifndef MOD_PLAYERBOTS_BOT_LOGIN_ACCOUNT_ROW_H
#define MOD_PLAYERBOTS_BOT_LOGIN_ACCOUNT_ROW_H

#include "Common.h"
#include "DatabaseEnv.h"

#include <algorithm>
#include <string>

namespace Playerbots
{
/// Loads username, merged GM security level, and expansion for constructing a WorldSession (LOGIN_SEL_ACCOUNT_INFO).
inline bool LoadLoginAccountRowForWorldSession(uint32 accountId, std::string& usernameOut, AccountTypes& secOut, uint8& expansionOut,
                                               std::string& err)
{
    LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_INFO);
    stmt->setUInt32(0, accountId);
    PreparedQueryResult result = LoginDatabase.Query(stmt);
    if (!result)
    {
        err = "Account not found in login database";
        return false;
    }

    uint8 maxGm = 0;
    bool first = true;
    do
    {
        Field* f = result->Fetch();
        if (first)
        {
            usernameOut = f[0].GetString();
            expansionOut = f[3].GetUInt8();
            first = false;
        }
        maxGm = std::max(maxGm, f[2].GetUInt8());
    } while (result->NextRow());

    secOut = AccountTypes(maxGm);
    return true;
}
} // namespace Playerbots

#endif
