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

private:
    static bool _enabled;
    static bool _logOnStartup;
    static std::string _accountPrefix;
    static uint32 _accountRandomSuffixLength;
    static uint32 _maxAccountCreateAttempts;
    static uint32 _maxCharacterCreateAttempts;
    static bool _autoLoginOnStartup;
    static uint32 _devBatchMax;
};
} // namespace Playerbots

#endif
