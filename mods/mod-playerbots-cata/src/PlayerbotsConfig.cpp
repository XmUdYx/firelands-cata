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

#include "PlayerbotsConfig.h"

// Config.h uses std::length_error before the standard header is included; mods compile without common PCH.
#include <stdexcept>

#include "Configuration/Config.h"

namespace Playerbots
{
bool Config::_enabled = false;
bool Config::_logOnStartup = true;
std::string Config::_accountPrefix = "PB";
uint32 Config::_accountRandomSuffixLength = 8;
uint32 Config::_maxAccountCreateAttempts = 32;
uint32 Config::_maxCharacterCreateAttempts = 48;
bool Config::_autoLoginOnStartup = false;
uint32 Config::_devBatchMax = 50;

bool Config::_idleMovementEnabled = true;
uint32 Config::_idleUpdateIntervalMs = 250;
uint32 Config::_idleMinActionDelayMs = 4000;
uint32 Config::_idleMaxActionDelayMs = 14000;
uint32 Config::_idlePostMovePauseMinMs = 2500;
uint32 Config::_idlePostMovePauseMaxMs = 8000;
uint32 Config::_idleDistractMinMs = 2500;
uint32 Config::_idleDistractMaxMs = 6000;
float Config::_idleWanderRadiusYards = 12.0f;
float Config::_idleMinStepYards = 2.0f;
float Config::_idleMaxStepYards = 7.0f;
uint32 Config::_idleChanceFacePct = 35;
uint32 Config::_idleChanceDistractPct = 25;

bool Config::_targetSelectionEnabled = true;
uint32 Config::_targetSelectionUpdateIntervalMs = 500;
float Config::_targetSelectionSearchRadius = 25.0f;
uint8 Config::_targetSelectionMaxLevelDelta = 5;
bool Config::_targetSelectionSkipElites = true;
bool Config::_targetSelectionSkipBossFlag = true;
bool Config::_targetSelectionSkipDungeonBoss = true;
bool Config::_targetSelectionSkipCritters = true;
bool Config::_targetSelectionSkipCombatWithOthers = true;
bool Config::_targetSelectionRequireLos = false;

void Config::Reload()
{
    _enabled = sConfigMgr->GetBoolDefault("Playerbots.Enable", false);
    _logOnStartup = sConfigMgr->GetBoolDefault("Playerbots.LogOnStartup", true);
    _accountPrefix = sConfigMgr->GetStringDefault("Playerbots.AccountPrefix", "PB");
    _accountRandomSuffixLength = sConfigMgr->GetIntDefault("Playerbots.AccountRandomSuffixLength", 8);
    _maxAccountCreateAttempts = sConfigMgr->GetIntDefault("Playerbots.MaxAccountCreateAttempts", 32);
    _maxCharacterCreateAttempts = sConfigMgr->GetIntDefault("Playerbots.MaxCharacterCreateAttempts", 48);
    if (_accountRandomSuffixLength > 12)
        _accountRandomSuffixLength = 12;
    if (_maxAccountCreateAttempts < 1)
        _maxAccountCreateAttempts = 1;
    if (_maxCharacterCreateAttempts < 1)
        _maxCharacterCreateAttempts = 1;

    _autoLoginOnStartup = sConfigMgr->GetBoolDefault("Playerbots.AutoLoginOnStartup", false);
    _devBatchMax = sConfigMgr->GetIntDefault("Playerbots.DevBatchMax", 50);
    if (_devBatchMax < 1)
        _devBatchMax = 1;
    if (_devBatchMax > 500)
        _devBatchMax = 500;

    _idleMovementEnabled = sConfigMgr->GetBoolDefault("Playerbots.IdleMovement.Enable", true);
    _idleUpdateIntervalMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.UpdateIntervalMs", 250);
    _idleMinActionDelayMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.MinActionDelayMs", 4000);
    _idleMaxActionDelayMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.MaxActionDelayMs", 14000);
    _idlePostMovePauseMinMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.PostMovePauseMinMs", 2500);
    _idlePostMovePauseMaxMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.PostMovePauseMaxMs", 8000);
    _idleDistractMinMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.DistractMinMs", 2500);
    _idleDistractMaxMs = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.DistractMaxMs", 6000);
    _idleWanderRadiusYards = sConfigMgr->GetFloatDefault("Playerbots.IdleMovement.WanderRadiusYards", 12.0f);
    _idleMinStepYards = sConfigMgr->GetFloatDefault("Playerbots.IdleMovement.MinStepYards", 2.0f);
    _idleMaxStepYards = sConfigMgr->GetFloatDefault("Playerbots.IdleMovement.MaxStepYards", 7.0f);
    _idleChanceFacePct = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.ChanceFacePct", 35);
    _idleChanceDistractPct = sConfigMgr->GetIntDefault("Playerbots.IdleMovement.ChanceDistractPct", 25);

    if (_idleUpdateIntervalMs < 100)
        _idleUpdateIntervalMs = 100;
    if (_idleUpdateIntervalMs > 2000)
        _idleUpdateIntervalMs = 2000;
    if (_idleMinActionDelayMs < 500)
        _idleMinActionDelayMs = 500;
    if (_idleMaxActionDelayMs < _idleMinActionDelayMs)
        _idleMaxActionDelayMs = _idleMinActionDelayMs + 1000;
    if (_idlePostMovePauseMinMs < 500)
        _idlePostMovePauseMinMs = 500;
    if (_idlePostMovePauseMaxMs < _idlePostMovePauseMinMs)
        _idlePostMovePauseMaxMs = _idlePostMovePauseMinMs + 500;
    if (_idleDistractMinMs < 1000)
        _idleDistractMinMs = 1000;
    if (_idleDistractMaxMs < _idleDistractMinMs)
        _idleDistractMaxMs = _idleDistractMinMs + 500;
    if (_idleWanderRadiusYards < 3.0f)
        _idleWanderRadiusYards = 3.0f;
    if (_idleWanderRadiusYards > 40.0f)
        _idleWanderRadiusYards = 40.0f;
    if (_idleMinStepYards < 0.5f)
        _idleMinStepYards = 0.5f;
    if (_idleMaxStepYards < _idleMinStepYards)
        _idleMaxStepYards = _idleMinStepYards + 0.5f;
    if (_idleMaxStepYards > _idleWanderRadiusYards)
        _idleMaxStepYards = _idleWanderRadiusYards;
    if (_idleChanceFacePct > 100)
        _idleChanceFacePct = 100;
    if (_idleChanceDistractPct > 100)
        _idleChanceDistractPct = 100;
    if (_idleChanceFacePct + _idleChanceDistractPct > 100)
        _idleChanceDistractPct = 100 - _idleChanceFacePct;

    _targetSelectionEnabled = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.Enable", true);
    _targetSelectionUpdateIntervalMs = sConfigMgr->GetIntDefault("Playerbots.TargetSelection.UpdateIntervalMs", 500);
    _targetSelectionSearchRadius = sConfigMgr->GetFloatDefault("Playerbots.TargetSelection.SearchRadiusYards", 25.0f);
    _targetSelectionMaxLevelDelta = static_cast<uint8>(sConfigMgr->GetIntDefault("Playerbots.TargetSelection.MaxLevelDelta", 5));
    _targetSelectionSkipElites = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.SkipElites", true);
    _targetSelectionSkipBossFlag = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.SkipBossFlag", true);
    _targetSelectionSkipDungeonBoss = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.SkipDungeonBoss", true);
    _targetSelectionSkipCritters = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.SkipCritters", true);
    _targetSelectionSkipCombatWithOthers = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.SkipCombatWithOthers", true);
    _targetSelectionRequireLos = sConfigMgr->GetBoolDefault("Playerbots.TargetSelection.RequireLos", false);

    if (_targetSelectionUpdateIntervalMs < 100)
        _targetSelectionUpdateIntervalMs = 100;
    if (_targetSelectionUpdateIntervalMs > 5000)
        _targetSelectionUpdateIntervalMs = 5000;
    if (_targetSelectionSearchRadius < 5.0f)
        _targetSelectionSearchRadius = 5.0f;
    if (_targetSelectionSearchRadius > 60.0f)
        _targetSelectionSearchRadius = 60.0f;
    if (_targetSelectionMaxLevelDelta > 20)
        _targetSelectionMaxLevelDelta = 20;
}

bool Config::IsEnabled()
{
    return _enabled;
}

bool Config::LogOnStartup()
{
    return _logOnStartup;
}

std::string const& Config::GetAccountPrefix()
{
    return _accountPrefix;
}

uint32 Config::GetAccountRandomSuffixLength()
{
    return _accountRandomSuffixLength;
}

uint32 Config::GetMaxAccountCreateAttempts()
{
    return _maxAccountCreateAttempts;
}

uint32 Config::GetMaxCharacterCreateAttempts()
{
    return _maxCharacterCreateAttempts;
}

bool Config::AutoLoginOnStartup()
{
    return _autoLoginOnStartup;
}

uint32 Config::GetDevBatchMax()
{
    return _devBatchMax;
}

bool Config::IsIdleMovementEnabled()
{
    return _idleMovementEnabled;
}

uint32 Config::GetIdleUpdateIntervalMs()
{
    return _idleUpdateIntervalMs;
}

uint32 Config::GetIdleMinActionDelayMs()
{
    return _idleMinActionDelayMs;
}

uint32 Config::GetIdleMaxActionDelayMs()
{
    return _idleMaxActionDelayMs;
}

uint32 Config::GetIdlePostMovePauseMinMs()
{
    return _idlePostMovePauseMinMs;
}

uint32 Config::GetIdlePostMovePauseMaxMs()
{
    return _idlePostMovePauseMaxMs;
}

uint32 Config::GetIdleDistractMinMs()
{
    return _idleDistractMinMs;
}

uint32 Config::GetIdleDistractMaxMs()
{
    return _idleDistractMaxMs;
}

float Config::GetIdleWanderRadiusYards()
{
    return _idleWanderRadiusYards;
}

float Config::GetIdleMinStepYards()
{
    return _idleMinStepYards;
}

float Config::GetIdleMaxStepYards()
{
    return _idleMaxStepYards;
}

uint32 Config::GetIdleChanceFacePct()
{
    return _idleChanceFacePct;
}

uint32 Config::GetIdleChanceDistractPct()
{
    return _idleChanceDistractPct;
}

bool Config::IsTargetSelectionEnabled()
{
    return _targetSelectionEnabled;
}

uint32 Config::GetTargetSelectionUpdateIntervalMs()
{
    return _targetSelectionUpdateIntervalMs;
}

float Config::GetTargetSelectionSearchRadius()
{
    return _targetSelectionSearchRadius;
}

uint8 Config::GetTargetSelectionMaxLevelDelta()
{
    return _targetSelectionMaxLevelDelta;
}

bool Config::IsTargetSelectionSkipElites()
{
    return _targetSelectionSkipElites;
}

bool Config::IsTargetSelectionSkipBossFlag()
{
    return _targetSelectionSkipBossFlag;
}

bool Config::IsTargetSelectionSkipDungeonBoss()
{
    return _targetSelectionSkipDungeonBoss;
}

bool Config::IsTargetSelectionSkipCritters()
{
    return _targetSelectionSkipCritters;
}

bool Config::IsTargetSelectionSkipCombatWithOthers()
{
    return _targetSelectionSkipCombatWithOthers;
}

bool Config::IsTargetSelectionRequireLos()
{
    return _targetSelectionRequireLos;
}
} // namespace Playerbots
