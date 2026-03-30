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

#include "BotCombatLoopService.h"
#include "BotIdleMovementService.h"
#include "BotLoginCoordinator.h"
#include "BotTargetSelectionService.h"
#include "PlayerbotsConfig.h"
#include "PlayerbotsTalentBuildStore.h"

#include "Player.h"
#include "ScriptMgr.h"
#include "WorldSession.h"

namespace
{
class PlayerbotsPlayerScript : public PlayerScript
{
public:
    PlayerbotsPlayerScript() : PlayerScript("PlayerbotsPlayerScript") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (!Playerbots::Config::IsEnabled() || !player || !player->GetSession())
            return;
        if (!player->GetSession()->IsHeadlessBotSession())
            return;

        Playerbots::BotLoginCoordinator::Instance().RegisterManagedBot(player->GetSession()->GetAccountId());
        Playerbots::PlayerbotsTalentBuildStore::OnBotEnteredWorld(player);
    }

    void OnLogout(Player* player) override
    {
        if (!player || !player->GetSession())
            return;
        if (!player->GetSession()->IsHeadlessBotSession())
            return;

        Playerbots::BotIdleMovementService::Instance().OnPlayerLogout(player);
        Playerbots::BotTargetSelectionService::Instance().OnPlayerLogout(player);
        Playerbots::BotCombatLoopService::Instance().OnPlayerLogout(player);
        Playerbots::BotLoginCoordinator::Instance().UnregisterManagedBot(player->GetSession()->GetAccountId());
    }

    void OnUpdate(Player* player, uint32 diff) override
    {
        if (!Playerbots::Config::IsEnabled())
            return;
        Playerbots::BotTargetSelectionService::Instance().OnPlayerUpdate(player, diff);
        Playerbots::BotCombatLoopService::Instance().OnPlayerUpdate(player, diff);
        Playerbots::BotIdleMovementService::Instance().OnPlayerUpdate(player, diff);
    }
};
} // namespace

void AddSC_playerbots_player_script()
{
    new PlayerbotsPlayerScript();
}
