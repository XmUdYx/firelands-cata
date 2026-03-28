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

#ifndef _PLAYERBOTS_TALENT_BUILD_STORE_H_
#define _PLAYERBOTS_TALENT_BUILD_STORE_H_

class Player;

namespace Playerbots
{
/// Cataclysm talent build drafts: load/save and future application to Player (implementation: later phase).
class PlayerbotsTalentBuildStore
{
public:
    PlayerbotsTalentBuildStore() = default;

    /// Hook point after a headless bot reaches world; no-op until talent automation exists.
    static void OnBotEnteredWorld(Player* /*player*/);
};
} // namespace Playerbots

#endif
