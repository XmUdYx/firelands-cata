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

#ifndef _PLAYERBOTS_CONFIG_H_
#define _PLAYERBOTS_CONFIG_H_

#include <string>

#include "Define.h"

namespace Playerbots
{
/// Reads module keys from ConfigMgr. Call from WorldScript::OnConfigLoad (and after module reload).
class Config
{
public:
    static void Reload();

    static bool IsEnabled();
    static bool LogOnStartup();

    static std::string const& GetAccountPrefix();
    static uint32 GetAccountRandomSuffixLength();
    static uint32 GetMaxAccountCreateAttempts();
    static uint32 GetMaxCharacterCreateAttempts();

    static bool AutoLoginOnStartup();

    /// Hard cap for `.playerbots batch` (dev safety).
    static uint32 GetDevBatchMax();

    /// Phase 2: idle / random nearby wander (managed headless bots only).
    static bool IsIdleMovementEnabled();
    static uint32 GetIdleUpdateIntervalMs();
    static uint32 GetIdleMinActionDelayMs();
    static uint32 GetIdleMaxActionDelayMs();
    static uint32 GetIdlePostMovePauseMinMs();
    static uint32 GetIdlePostMovePauseMaxMs();
    static uint32 GetIdleDistractMinMs();
    static uint32 GetIdleDistractMaxMs();
    static float GetIdleWanderRadiusYards();
    static float GetIdleMinStepYards();
    static float GetIdleMaxStepYards();
    static uint32 GetIdleChanceFacePct();
    static uint32 GetIdleChanceDistractPct();

    /// Phase 2: simple hostile creature target selection (no combat rotation).
    static bool IsTargetSelectionEnabled();
    static uint32 GetTargetSelectionUpdateIntervalMs();
    static float GetTargetSelectionSearchRadius();
    static uint8 GetTargetSelectionMaxLevelDelta();
    static bool IsTargetSelectionSkipElites();
    static bool IsTargetSelectionSkipBossFlag();
    static bool IsTargetSelectionSkipDungeonBoss();
    static bool IsTargetSelectionSkipCritters();
    static bool IsTargetSelectionSkipCombatWithOthers();
    static bool IsTargetSelectionRequireLos();

private:
    static bool _enabled;
    static bool _logOnStartup;
    static std::string _accountPrefix;
    static uint32 _accountRandomSuffixLength;
    static uint32 _maxAccountCreateAttempts;
    static uint32 _maxCharacterCreateAttempts;
    static bool _autoLoginOnStartup;
    static uint32 _devBatchMax;

    static bool _idleMovementEnabled;
    static uint32 _idleUpdateIntervalMs;
    static uint32 _idleMinActionDelayMs;
    static uint32 _idleMaxActionDelayMs;
    static uint32 _idlePostMovePauseMinMs;
    static uint32 _idlePostMovePauseMaxMs;
    static uint32 _idleDistractMinMs;
    static uint32 _idleDistractMaxMs;
    static float _idleWanderRadiusYards;
    static float _idleMinStepYards;
    static float _idleMaxStepYards;
    static uint32 _idleChanceFacePct;
    static uint32 _idleChanceDistractPct;

    static bool _targetSelectionEnabled;
    static uint32 _targetSelectionUpdateIntervalMs;
    static float _targetSelectionSearchRadius;
    static uint8 _targetSelectionMaxLevelDelta;
    static bool _targetSelectionSkipElites;
    static bool _targetSelectionSkipBossFlag;
    static bool _targetSelectionSkipDungeonBoss;
    static bool _targetSelectionSkipCritters;
    static bool _targetSelectionSkipCombatWithOthers;
    static bool _targetSelectionRequireLos;
};
} // namespace Playerbots

#endif
