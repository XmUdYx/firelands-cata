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

#include "BotAccountService.h"
#include "PlayerbotsConfig.h"

#include "AccountMgr.h"
#include "Log.h"
#include "Random.h"
#include "Util.h"

namespace Playerbots
{
namespace
{
std::string MakeRandomPassword()
{
    static char const* hex = "0123456789ABCDEF";
    std::string s;
    s.reserve(MAX_PASS_STR);
    for (uint32 i = 0; i < MAX_PASS_STR; ++i)
        s.push_back(hex[urand(0, 15)]);
    return s;
}

std::string BuildCandidateUsername(std::string const& prefix, uint32 suffixLen)
{
    std::string base = prefix;
    if (!base.empty() && base.back() != '_')
        base.push_back('_');

    static char const* hex = "0123456789ABCDEF";
    for (uint32 i = 0; i < suffixLen; ++i)
        base.push_back(hex[urand(0, 15)]);

    Utf8ToUpperOnlyLatin(base);
    return base;
}
} // namespace

bool BotAccountService::CreateBotAccount(uint32& accountIdOut, std::string& usernameOut, std::string& passwordPlainOut, std::string& err)
{
    accountIdOut = 0;
    usernameOut.clear();
    passwordPlainOut = MakeRandomPassword();

    uint32 const maxAttempts = Config::GetMaxAccountCreateAttempts();
    uint32 suffixLen = Config::GetAccountRandomSuffixLength();
    if (suffixLen < 4)
        suffixLen = 4;

    for (uint32 attempt = 0; attempt < maxAttempts; ++attempt)
    {
        std::string candidate = BuildCandidateUsername(Config::GetAccountPrefix(), suffixLen);
        if (utf8length(candidate) > MAX_ACCOUNT_STR)
        {
            if (suffixLen > 4)
            {
                --suffixLen;
                continue;
            }
            err = "Account name prefix/suffix configuration exceeds max account name length";
            return false;
        }

        AccountOpResult const r = AccountMgr::instance()->CreateAccount(candidate, passwordPlainOut);
        if (r == AccountOpResult::AOR_NAME_ALREADY_EXIST)
            continue;
        if (r != AccountOpResult::AOR_OK)
        {
            switch (r)
            {
            case AccountOpResult::AOR_NAME_TOO_LONG:
                err = "Account name too long";
                break;
            case AccountOpResult::AOR_PASS_TOO_LONG:
                err = "Generated password too long (internal error)";
                break;
            default:
                err = "Account creation failed";
                break;
            }
            LOG_ERROR("misc", "mod-playerbots: CreateAccount failed (code %u)", uint32(r));
            return false;
        }

        Utf8ToUpperOnlyLatin(candidate);
        uint32 const id = AccountMgr::GetId(candidate);
        if (!id)
        {
            err = "Account created but id lookup failed";
            return false;
        }

        accountIdOut = id;
        usernameOut = std::move(candidate);
        return true;
    }

    err = "Could not allocate a unique bot account name";
    return false;
}
} // namespace Playerbots
