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
#include "BotPersistenceRepository.h"
#include "PlayerbotsConfig.h"
#include "Define.h"
#include "Log.h"
#include "ScriptMgr.h"

namespace
{
class PlayerbotsWorldScript : public WorldScript
{
public:
    PlayerbotsWorldScript() : WorldScript("PlayerbotsWorldScript") { }

    void OnConfigLoad(bool /*reload*/) override { Playerbots::Config::Reload(); }

    void OnStartup() override
    {
        if (!Playerbots::Config::IsEnabled() || !Playerbots::Config::LogOnStartup())
            return;

        Playerbots::BotPersistenceRepository repo;
        LOG_INFO("server.loading", "mod-playerbots-cata: enabled, `playerbots_bot` rows: " UI64FMTD, repo.CountBots());
    }

    void OnUpdate(uint32 /*diff*/) override
    {
        if (_ranDeferredAutoLogin)
            return;

        if (!Playerbots::Config::IsEnabled() || !Playerbots::Config::AutoLoginOnStartup())
        {
            _ranDeferredAutoLogin = true;
            return;
        }

        // Few ticks after world start so character cache / maps are ready before headless logins.
        if (++_startupDelayTicks < 15)
            return;

        _ranDeferredAutoLogin = true;
        Playerbots::BotLoginCoordinator::Instance().LoginAllPersistedBots();
    }

private:
    uint32 _startupDelayTicks = 0;
    bool _ranDeferredAutoLogin = false;
};
} // namespace

void AddSC_playerbots_world_script()
{
    new PlayerbotsWorldScript();
}
