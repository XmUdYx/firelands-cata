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
#include "BotCharacterService.h"
#include "BotPersistenceRepository.h"
#include "BotSessionBootstrap.h"
#include "PlayerbotsConfig.h"

#include "Chat.h"
#include "Define.h"
#include "ObjectGuid.h"
#include "RBAC.h"
#include "ScriptMgr.h"

namespace
{
bool EnsurePlayerbotsEnabled(ChatHandler* handler)
{
    if (!Playerbots::Config::IsEnabled())
    {
        handler->PSendSysMessage("Playerbots: module disabled (set Playerbots.Enable = 1).");
        return false;
    }
    return true;
}

class playerbots_commandscript : public CommandScript
{
public:
    playerbots_commandscript() : CommandScript("playerbots_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> loginTable = {
            { "all", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsLoginAll, "" },
        };

        static std::vector<ChatCommand> logoutTable = {
            { "all", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsLogoutAll, "" },
        };

        static std::vector<ChatCommand> playerbotsSubcommands = {
            { "help", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsHelp, "" },
            { "status", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsStatus, "" },
            { "batch", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsBatch, "" },
            { "create", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsCreate, "" },
            { "login", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, nullptr, "", loginTable },
            { "logout", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, nullptr, "", logoutTable },
            { "", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, &HandlePlayerbotsHelp, "" },
        };

        static std::vector<ChatCommand> commandTable = {
            { "playerbots", rbac::RBAC_PERM_COMMAND_ACCOUNT_CREATE, true, nullptr, "", playerbotsSubcommands },
        };
        return commandTable;
    }

    static bool HandlePlayerbotsHelp(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage("== playerbots (Phase 1 dev) ==");
        handler->PSendSysMessage(".playerbots status — DB totals, login-eligible rows, managed headless count.");
        handler->PSendSysMessage(".playerbots batch <n> — create n random bot accounts+chars (cap: Playerbots.DevBatchMax).");
        handler->PSendSysMessage(".playerbots create — create one bot (prints password once).");
        handler->PSendSysMessage(".playerbots login all — queue headless login for every active persisted bot.");
        handler->PSendSysMessage(".playerbots logout all — save+logout managed headless bots.");
        handler->PSendSysMessage("Config: Playerbots.Enable, PlayerbotsDatabase*, Playerbots.AutoLoginOnStartup, Playerbots.DevBatchMax.");
        return true;
    }

    static bool HandlePlayerbotsStatus(ChatHandler* handler, char const* /*args*/)
    {
        handler->PSendSysMessage("Playerbots.Enable = %s", Playerbots::Config::IsEnabled() ? "1" : "0");
        handler->PSendSysMessage("Playerbots.AutoLoginOnStartup = %s", Playerbots::Config::AutoLoginOnStartup() ? "1" : "0");
        handler->PSendSysMessage("Playerbots.DevBatchMax = %u", Playerbots::Config::GetDevBatchMax());

        Playerbots::BotPersistenceRepository repo;
        uint64 const totalRows = repo.CountBots();
        uint64 const activeRows = repo.CountActivePersistedBots();
        size_t const managed = Playerbots::BotSessionBootstrap::GetManagedHeadlessBotCount();

        handler->PSendSysMessage("playerbots_bot rows (all statuses): " UI64FMTD, totalRows);
        handler->PSendSysMessage("playerbots_bot rows (active+bound char): " UI64FMTD, activeRows);
        handler->PSendSysMessage("managed headless accounts (in world since last login): " UI64FMTD, uint64(managed));
        return true;
    }

    static bool HandlePlayerbotsBatch(ChatHandler* handler, uint32 count)
    {
        if (!EnsurePlayerbotsEnabled(handler))
            return true;

        uint32 const cap = Playerbots::Config::GetDevBatchMax();
        if (count == 0 || count > cap)
        {
            handler->PSendSysMessage("Playerbots: batch count must be 1..%u (got %u).", cap, count);
            return true;
        }

        uint32 ok = 0, fail = 0;
        std::string lastErr;

        for (uint32 i = 0; i < count; ++i)
        {
            uint32 accountId = 0;
            std::string username;
            std::string password;
            std::string err;

            if (!Playerbots::BotAccountService::CreateBotAccount(accountId, username, password, err))
            {
                lastErr = err;
                ++fail;
                continue;
            }

            ObjectGuid::LowType guidLow = 0;
            std::string charName;
            if (!Playerbots::BotCharacterService::CreateFirstRandomBotCharacter(accountId, guidLow, charName, err))
            {
                lastErr = err;
                ++fail;
                continue;
            }

            ++ok;
            handler->PSendSysMessage("Playerbots: [%u/%u] ok account=%u user=%s char=%s lowguid=%u", i + 1, count, accountId, username.c_str(),
                charName.c_str(), guidLow);
        }

        handler->PSendSysMessage("Playerbots: batch done — success %u, failed %u (of %u).", ok, fail, count);
        if (fail && !lastErr.empty())
            handler->PSendSysMessage("Playerbots: last error: %s", lastErr.c_str());

        return true;
    }

    static bool HandlePlayerbotsLoginAll(ChatHandler* handler, char const* /*args*/)
    {
        if (!EnsurePlayerbotsEnabled(handler))
            return true;

        Playerbots::LoginAllPersistedResult const r = Playerbots::BotSessionBootstrap::LoginAllPersistedBotsWithResult();
        if (!r.lastError.empty() && r.totalRows == 0)
            handler->PSendSysMessage("Playerbots: login all — %s", r.lastError.c_str());
        else
        {
            handler->PSendSysMessage("Playerbots: login all — rows=%u, queued_ok=%u, failed=%u.", r.totalRows, r.queuedOk, r.failed);
            if (r.failed && !r.lastError.empty())
                handler->PSendSysMessage("Playerbots: last failure reason: %s", r.lastError.c_str());
        }
        return true;
    }

    static bool HandlePlayerbotsLogoutAll(ChatHandler* handler, char const* /*args*/)
    {
        if (!EnsurePlayerbotsEnabled(handler))
            return true;

        uint32 const n = Playerbots::BotSessionBootstrap::LogoutAllManagedBotsWithCount();
        handler->PSendSysMessage("Playerbots: logout all — issued logout for %u in-world managed bot(s).", n);
        return true;
    }

    static bool HandlePlayerbotsCreate(ChatHandler* handler, char const* /*args*/)
    {
        if (!EnsurePlayerbotsEnabled(handler))
            return true;

        uint32 accountId = 0;
        std::string username;
        std::string password;
        std::string err;

        if (!Playerbots::BotAccountService::CreateBotAccount(accountId, username, password, err))
        {
            handler->PSendSysMessage("Playerbots: account error: %s", err.c_str());
            return true;
        }

        ObjectGuid::LowType guidLow = 0;
        std::string charName;
        if (!Playerbots::BotCharacterService::CreateFirstRandomBotCharacter(accountId, guidLow, charName, err))
        {
            handler->PSendSysMessage("Playerbots: character error (account %u '%s'): %s", accountId, username.c_str(), err.c_str());
            return true;
        }

        handler->PSendSysMessage("Playerbots: created account '%s' (id %u), password '%s', character '%s' (lowguid %u).", username.c_str(),
            accountId, password.c_str(), charName.c_str(), guidLow);
        return true;
    }
};
} // namespace

void AddSC_playerbots_commandscript()
{
    new playerbots_commandscript();
}
