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

#include "BotCharacterService.h"
#include "BotLoginAccountRow.h"
#include "BotPersistenceRepository.h"
#include "PlayerbotsConfig.h"

#include "AccountMgr.h"
#include "CharacterCache.h"
#include "Common.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "Transaction.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Random.h"
#include "RBAC.h"
#include "Realm.h"
#include "ScriptMgr.h"
#include "SharedDefines.h"
#include "World.h"
#include "WorldSession.h"

#include <memory>
#include <utility>
#include <vector>

namespace Playerbots
{
namespace
{
void CollectValidRaceClassPairs(WorldSession& session, std::vector<std::pair<uint8, uint8>>& out)
{
    uint32 const teamMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED);
    uint32 const raceMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK);
    uint32 const classMaskDisabled = sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK);
    uint8 const accExp = session.GetAccountExpansion();

    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
    {
        if (cls == CLASS_DEATH_KNIGHT)
            continue;

        if (!session.HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_CLASSMASK))
        {
            if ((1u << (cls - 1)) & classMaskDisabled)
                continue;
        }

        ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(cls);
        if (!cEntry || cEntry->Required_expansion > accExp)
            continue;

        for (uint8 race = 1; race < MAX_RACES; ++race)
        {
            if (!Player::IsValidRace(race))
                continue;
            if (!sObjectMgr->GetPlayerInfo(race, cls))
                continue;

            if (!session.HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_RACEMASK))
            {
                if ((1u << (race - 1)) & raceMaskDisabled)
                    continue;
            }

            ChrRacesEntry const* rEntry = sChrRacesStore.LookupEntry(race);
            if (!rEntry || rEntry->Race_related > accExp)
                continue;

            if (!session.HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_TEAMMASK))
            {
                bool disabled = false;
                switch (Player::TeamForRace(race))
                {
                case ALLIANCE:
                    disabled = (teamMaskDisabled & (1u << 0)) != 0;
                    break;
                case HORDE:
                    disabled = (teamMaskDisabled & (1u << 1)) != 0;
                    break;
                default:
                    break;
                }
                if (disabled)
                    continue;
            }

            out.emplace_back(race, cls);
        }
    }
}
} // namespace

bool BotCharacterService::CreateFirstRandomBotCharacter(uint32 accountId, ObjectGuid::LowType& outGuidLow, std::string& characterNameOut,
                                                        std::string& err)
{
    outGuidLow = 0;
    characterNameOut.clear();

    if (AccountMgr::GetCharactersCount(accountId) != 0)
    {
        err = "Account already has characters";
        return false;
    }

    BotPersistenceRepository persistence;
    if (persistence.HasBotForAccount(accountId))
    {
        err = "Playerbots row already exists for this account";
        return false;
    }

    std::string username;
    AccountTypes sec = SEC_PLAYER;
    uint8 expansion = 0;
    if (!LoadLoginAccountRowForWorldSession(accountId, username, sec, expansion, err))
        return false;

    WorldSession session(accountId, std::move(username), nullptr, sec, expansion, 0, LOCALE_enUS, 0, false);
    session.LoadPermissions();

    std::vector<std::pair<uint8, uint8>> pairs;
    CollectValidRaceClassPairs(session, pairs);
    if (pairs.empty())
    {
        err = "No race/class combination allowed by server rules and account expansion";
        return false;
    }

    {
        // LOGIN_SEL_SUM_REALM_CHARACTERS is CONNECTION_ASYNC in core; LoginDatabase::Query uses the sync pool only.
        QueryResult sumRes = LoginDatabase.PQuery("SELECT SUM(numchars) FROM realmcharacters WHERE acctid = %u", accountId);
        uint64 acctCharCount = 0;
        if (sumRes)
            acctCharCount = uint64(sumRes->Fetch()[0].GetDouble());
        if (acctCharCount >= uint64(sWorld->getIntConfig(CONFIG_CHARACTERS_PER_ACCOUNT)))
        {
            err = "Account character limit reached";
            return false;
        }
    }

    uint8 realmCharCount = 0;
    {
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_SUM_CHARS);
        stmt->setUInt32(0, accountId);
        PreparedQueryResult res = CharacterDatabase.Query(stmt);
        if (res)
            realmCharCount = uint8(res->Fetch()[0].GetUInt64());
        if (realmCharCount >= sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM))
        {
            err = "Realm character limit reached";
            return false;
        }
    }

    uint32 const maxAttempts = Config::GetMaxCharacterCreateAttempts();
    for (uint32 attempt = 0; attempt < maxAttempts; ++attempt)
    {
        std::pair<uint8, uint8> const rc = pairs[urand(0, uint32(pairs.size() - 1))];
        uint8 const race = rc.first;
        uint8 const class_ = rc.second;
        uint8 const gender = urand(0, 1) ? GENDER_MALE : GENDER_FEMALE;

        if (!Player::IsValidGender(gender))
            continue;

        std::string name = sDBCManager.GetRandomCharacterName(race, gender);
        if (!normalizePlayerName(name))
            continue;

        if (ObjectMgr::CheckPlayerName(name, session.GetSessionDbcLocale(), true) != CHAR_NAME_SUCCESS)
            continue;

        if (!session.HasPermission(rbac::RBAC_PERM_SKIP_CHECK_CHARACTER_CREATION_RESERVEDNAME) && sObjectMgr->IsReservedName(name))
            continue;

        {
            CharacterDatabasePreparedStatement* chk = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
            chk->setString(0, name);
            if (CharacterDatabase.Query(chk))
                continue;
        }

        if (sCharacterCache->GetCharacterCacheByName(name))
            continue;

        for (uint8 skin = 0; skin < 8; ++skin)
            for (uint8 face = 0; face < 8; ++face)
                for (uint8 hair = 0; hair < 12; ++hair)
                    for (uint8 hairc = 0; hairc < 8; ++hairc)
                        for (uint8 facial = 0; facial < 8; ++facial)
                        {
                            CharacterCreateInfo ci =
                                CharacterCreateInfo::CreateForInternalTool(name, race, class_, gender, skin, face, hair, hairc, facial);

                            ObjectGuid::LowType const guidLow = sObjectMgr->GetGenerator<HighGuid::Player>().Generate();

                            std::shared_ptr<Player> player(new Player(&session), [](Player* p) {
                                p->CleanupsBeforeDelete();
                                delete p;
                            });
                            player->GetMotionMaster()->Initialize();

                            if (!player->Create(guidLow, &ci))
                                continue;

                            uint32 const skipCinematics = sWorld->getIntConfig(CONFIG_SKIP_CINEMATICS);
                            bool const haveSameRace = false;
                            if ((haveSameRace && skipCinematics == 1) || skipCinematics == 2)
                                player->setCinematic(1);

                            player->SetAtLoginFlag(AT_LOGIN_FIRST);

                            // Player::SaveToDB uses CHAR_* statements that are CONNECTION_ASYNC in core.
                            // DirectCommitTransaction runs on the sync MySQL connection → null prepared slots → ASSERT.
                            // Same pattern as WorldSession character create: async commit on the character pool.
                            CharacterDatabaseTransaction charTr = CharacterDatabase.BeginTransaction();
                            player->SaveToDB(charTr, true);
                            TransactionCallback charCommit = CharacterDatabase.AsyncCommitTransaction(charTr);
                            if (!charCommit.m_future.get())
                                continue;

                            LoginDatabaseTransaction loTr = LoginDatabase.BeginTransaction();
                            LoginDatabasePreparedStatement* del = LoginDatabase.GetPreparedStatement(LOGIN_DEL_REALM_CHARACTERS_BY_REALM);
                            del->setUInt32(0, accountId);
                            del->setUInt32(1, realm.Id.Realm);
                            loTr->Append(del);

                            LoginDatabasePreparedStatement* ins = LoginDatabase.GetPreparedStatement(LOGIN_INS_REALM_CHARACTERS);
                            ins->setUInt32(0, uint32(realmCharCount) + 1);
                            ins->setUInt32(1, accountId);
                            ins->setUInt32(2, realm.Id.Realm);
                            loTr->Append(ins);
                            LoginDatabase.CommitTransaction(loTr);

                            ObjectGuid const fullGuid = player->GetGUID();
                            sScriptMgr->OnPlayerCreate(player.get());
                            sCharacterCache->AddCharacterCacheEntry(fullGuid, accountId, player->GetName(),
                                player->GetByteValue(PLAYER_BYTES_3, PLAYER_BYTES_3_OFFSET_GENDER), player->getRace(), player->getClass(),
                                player->getLevel());

                            persistence.InsertActiveBot(accountId, fullGuid.GetCounter(), player->GetName());

                            outGuidLow = guidLow;
                            characterNameOut = player->GetName();
                            return true;
                        }
    }

    err = "Failed to create a character after random retries (name/appearance collisions or rules)";
    return false;
}
} // namespace Playerbots
