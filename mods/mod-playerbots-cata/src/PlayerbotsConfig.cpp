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
} // namespace Playerbots
