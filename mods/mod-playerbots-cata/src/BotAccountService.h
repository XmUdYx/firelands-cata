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

#ifndef _PLAYERBOTS_BOT_ACCOUNT_SERVICE_H_
#define _PLAYERBOTS_BOT_ACCOUNT_SERVICE_H_

#include "Define.h"

#include <cstdint>
#include <string>

namespace Playerbots
{
/// Creates bot accounts via AccountMgr (auth DB), matching core validation rules.
class BotAccountService
{
public:
    /// Creates a new player account with a random password. `usernameOut` is the final stored login (uppercased).
    static bool CreateBotAccount(uint32& accountIdOut, std::string& usernameOut, std::string& passwordPlainOut, std::string& err);
};
} // namespace Playerbots

#endif
