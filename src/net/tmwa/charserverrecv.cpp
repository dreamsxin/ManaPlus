/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2015  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "net/tmwa/charserverrecv.h"

#include "client.h"
#include "configuration.h"
#include "settings.h"

#include "gui/windows/charcreatedialog.h"
#include "gui/windows/okdialog.h"

#include "gui/widgets/createwidget.h"

#include "net/character.h"

#include "net/ea/token.h"

#include "net/tmwa/gamehandler.h"
#include "net/tmwa/loginhandler.h"
#include "net/tmwa/messageout.h"
#include "net/tmwa/network.h"
#include "net/tmwa/protocol.h"
#include "net/tmwa/sprite.h"

#include "resources/iteminfo.h"

#include "resources/db/chardb.h"
#include "resources/db/itemdb.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include "debug.h"

extern Net::CharServerHandler *charServerHandler;

namespace TmwAthena
{

extern ServerInfo mapServer;

extern ServerInfo charServer;

void CharServerRecv::readPlayerData(Net::MessageIn &msg,
                                    Net::Character *const character)
{
    if (!character)
        return;

    const Token &token =
        static_cast<LoginHandler*>(loginHandler)->getToken();

    LocalPlayer *const tempPlayer = new LocalPlayer(
        msg.readBeingId("account id"), BeingTypeId_zero);
    tempPlayer->setGender(token.sex);

    PlayerInfoBackend &data = character->data;
    data.mAttributes[Attributes::EXP] = msg.readInt32("exp");
    data.mAttributes[Attributes::MONEY] = msg.readInt32("money");
    Stat &jobStat = data.mStats[Attributes::JOB];
    jobStat.exp = msg.readInt32("job");

    const int temp = msg.readInt32("job level");
    jobStat.base = temp;
    jobStat.mod = temp;

    const int shoes = msg.readInt16("shoes");
    const int gloves = msg.readInt16("gloves");
    const int cape = msg.readInt16("cape");
    const int misc1 = msg.readInt16("misc1");

    msg.readInt32("option");
    tempPlayer->setKarma(msg.readInt32("karma"));
    tempPlayer->setManner(msg.readInt32("manner"));
    msg.readInt16("character points left");

    data.mAttributes[Attributes::HP] = msg.readInt16("hp");
    data.mAttributes[Attributes::MAX_HP] = msg.readInt16("max hp");
    data.mAttributes[Attributes::MP] = msg.readInt16("mp");
    data.mAttributes[Attributes::MAX_MP] = msg.readInt16("max mp");

    msg.readInt16("speed");
    const uint16_t race = msg.readInt16("class");
    const uint8_t hairStyle = msg.readUInt8("hair style");
    const uint16_t look = msg.readUInt8("look");
    tempPlayer->setSubtype(fromInt(race, BeingTypeId), look);
    const uint16_t weapon = msg.readInt16("weapon");
    tempPlayer->setSprite(SPRITE_BODY, weapon, "", ItemColor_one, true);

    data.mAttributes[Attributes::LEVEL] = msg.readInt16("level");

    msg.readInt16("skill point");
    const int bottomClothes = msg.readInt16("bottom clothes");
    const int shield = msg.readInt16("shield");

    const int hat = msg.readInt16("hat");
    const int topClothes = msg.readInt16("top clothes");

    const ItemColor hairColor = fromInt(
        msg.readUInt8("hair color"), ItemColor);
    msg.readUInt8("unused");
    tempPlayer->setSprite(SPRITE_HAIR_COLOR, hairStyle * -1,
        ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
    tempPlayer->setHairColor(hairColor);

    const int misc2 = msg.readInt16("misc2");
    tempPlayer->setName(msg.readString(24, "name"));

    character->dummy = tempPlayer;

    character->data.mStats[Attributes::STR].base = msg.readUInt8("str");
    character->data.mStats[Attributes::AGI].base = msg.readUInt8("agi");
    character->data.mStats[Attributes::VIT].base = msg.readUInt8("vit");
    character->data.mStats[Attributes::INT].base = msg.readUInt8("int");
    character->data.mStats[Attributes::DEX].base = msg.readUInt8("dex");
    character->data.mStats[Attributes::LUK].base = msg.readUInt8("luk");

    tempPlayer->setSprite(SPRITE_HAIR, shoes);
    tempPlayer->setSprite(SPRITE_SHOES, gloves);
    tempPlayer->setSprite(SPRITE_SHIELD, cape);
    tempPlayer->setSprite(SPRITE_HEAD_TOP, misc1);
    tempPlayer->setSprite(SPRITE_WEAPON, bottomClothes);
    tempPlayer->setSprite(SPRITE_FLOOR, shield);
    tempPlayer->setSprite(SPRITE_CLOTHES_COLOR, hat);  // head option top
    tempPlayer->setSprite(SPRITE_HEAD_BOTTOM, topClothes);
    tempPlayer->setSprite(SPRITE_HEAD_MID, misc2);

    character->slot = msg.readUInt8("slot");
    msg.readUInt8("unused");
}

void CharServerRecv::processCharLogin(Net::MessageIn &msg)
{
    BLOCK_START("CharServerRecv::processCharLogin")

    msg.readInt16("len");
    const int slots = msg.readInt16("slots");
    if (slots > 0 && slots < 30)
        loginData.characterSlots = static_cast<uint16_t>(slots);

    msg.skip(18, "unused");

    delete_all(charServerHandler->mCharacters);
    charServerHandler->mCharacters.clear();

    // Derive number of characters from message length
    const int count = (msg.getLength() - 24) / 106;

    for (int i = 0; i < count; ++i)
    {
        Net::Character *const character = new Net::Character;
        readPlayerData(msg, character);
        charServerHandler->mCharacters.push_back(character);
        if (character->dummy)
        {
            logger->log("CharServer: Player: %s (%d)",
                character->dummy->getName().c_str(), character->slot);
        }
    }

    client->setState(STATE_CHAR_SELECT);
    BLOCK_END("CharServerRecv::processCharLogin")
}

void CharServerRecv::processCharMapInfo(Net::MessageIn &restrict msg)
{
    Network *const network = Network::mInstance;
    ServerInfo &server = mapServer;
    BLOCK_START("CharServerRecv::processCharMapInfo")
    PlayerInfo::setCharId(msg.readInt32("char id?"));
    GameHandler *const gh = static_cast<GameHandler*>(gameHandler);
    gh->setMap(msg.readString(16, "map name"));
    if (config.getBoolValue("usePersistentIP") || settings.persistentIp)
    {
        msg.readInt32("ip address");
        server.hostname = settings.serverName;
    }
    else
    {
        server.hostname = ipToString(msg.readInt32("ip address"));
    }
    server.port = msg.readInt16("port");

    // Prevent the selected local player from being deleted
    localPlayer = charServerHandler->mSelectedCharacter->dummy;
    PlayerInfo::setBackend(charServerHandler->mSelectedCharacter->data);

    charServerHandler->mSelectedCharacter->dummy = nullptr;

    charServerHandler->clear();
    charServerHandler->updateCharSelectDialog();

    if (network)
        network->disconnect();
    client->setState(STATE_CONNECT_GAME);
    BLOCK_END("CharServerRecv::processCharMapInfo")
}

void CharServerRecv::processChangeMapServer(Net::MessageIn &msg)
{
    Network *const network = Network::mInstance;
    ServerInfo &server = mapServer;
    BLOCK_START("CharServerRecv::processChangeMapServer")
    GameHandler *const gh = static_cast<GameHandler*>(gameHandler);
    if (!gh || !network)
    {
        BLOCK_END("CharServerRecv::processChangeMapServer")
        return;
    }
    gh->setMap(msg.readString(16, "map name"));
    const int x = msg.readInt16("x");
    const int y = msg.readInt16("y");
    if (config.getBoolValue("usePersistentIP") || settings.persistentIp)
    {
        msg.readInt32("ip address");
        server.hostname = settings.serverName;
    }
    else
    {
        server.hostname = ipToString(msg.readInt32("ip address"));
    }
    server.port = msg.readInt16("port");

    network->disconnect();
    client->setState(STATE_CHANGE_MAP);
    if (localPlayer)
    {
        localPlayer->setTileCoords(x, y);
        localPlayer->setMap(nullptr);
    }
    BLOCK_END("CharServerRecv::processChangeMapServer")
}

void CharServerRecv::processCharCreate(Net::MessageIn &msg)
{
    BLOCK_START("CharServerRecv::processCharCreate")
    Net::Character *const character = new Net::Character;
    readPlayerData(msg, character);
    charServerHandler->mCharacters.push_back(character);

    charServerHandler->updateCharSelectDialog();

    // Close the character create dialog
    if (charServerHandler->mCharCreateDialog)
    {
        charServerHandler->mCharCreateDialog->scheduleDelete();
        charServerHandler->mCharCreateDialog = nullptr;
    }
    BLOCK_END("CharServerRecv::processCharCreate")
}

void CharServerRecv::processCharDeleteFailed(Net::MessageIn &msg)
{
    BLOCK_START("CharServerRecv::processCharDeleteFailed")
    charServerHandler->unlockCharSelectDialog();
    msg.readUInt8("error");
    CREATEWIDGET(OkDialog,
        // TRANSLATORS: error header
        _("Error"),
        // TRANSLATORS: error message
        _("Failed to delete character."),
        // TRANSLATORS: ok dialog button
        _("OK"),
        DialogType::ERROR,
        Modal_true,
        ShowCenter_true,
        nullptr,
        260);
    BLOCK_END("CharServerRecv::processCharDeleteFailed")
}

}  // namespace TmwAthena