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

#include "net/eathena/beingrecv.h"

#include "actormanager.h"
#include "effectmanager.h"
#include "game.h"
#include "itemcolormanager.h"
#include "notifymanager.h"
#include "party.h"

#include "being/mercenaryinfo.h"

#include "enums/resources/notifytypes.h"

#include "particle/particle.h"

#include "input/keyboardconfig.h"

#include "gui/windows/skilldialog.h"
#include "gui/windows/socialwindow.h"
#include "gui/windows/outfitwindow.h"

#include "net/serverfeatures.h"

#include "net/ea/beingrecv.h"
#include "net/ea/eaprotocol.h"

#include "net/charserverhandler.h"

#include "net/character.h"

#include "net/eathena/maptypeproperty2.h"
#include "net/eathena/messageout.h"
#include "net/eathena/protocol.h"
#include "net/eathena/sprite.h"

#include "resources/iteminfo.h"
#include "resources/db/itemdb.h"

#include "resources/map/map.h"

#include "utils/timer.h"

#include "debug.h"

namespace EAthena
{

void BeingRecv::processBeingChangeLook2(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));
    const uint8_t type = msg.readUInt8("type");

    const int id = msg.readInt16("id1");
    unsigned int id2 = msg.readInt16("id2");
    if (type != 2)
        id2 = 1;

    if (!localPlayer || !dstBeing)
        return;

    processBeingChangeLookContinue(msg, dstBeing, type, id, id2, nullptr);
}

void BeingRecv::processBeingChangeLookCards(Net::MessageIn &msg)
{
    Being *dstBeing = nullptr;
    int cards[4];

    if (!actorManager)
    { // here can be look from char server
        if (!serverFeatures->haveAdvancedSprites())
            return;
        Net::Characters &chars = Net::CharServerHandler::mCharacters;
        const BeingId id = msg.readBeingId("being id");

        FOR_EACH (Net::Characters::iterator, it, chars)
        {
            Net::Character *character = *it;
            if (character->dummy && character->dummy->getId() == id)
            {
                dstBeing = character->dummy;
                break;
            }
        }
    }
    else
    {
        dstBeing = actorManager->findBeing(
            msg.readBeingId("being id"));
    }

    const uint8_t type = msg.readUInt8("type");

    const int id = msg.readInt16("id1");
    unsigned int id2 = msg.readInt16("id2");
    if (type != 2)
        id2 = 1;

    for (int f = 0; f < 4; f ++)
        cards[f] = msg.readInt16("card");

    if (!dstBeing)
        return;

    processBeingChangeLookContinue(msg, dstBeing, type, id, id2, &cards[0]);
}

void BeingRecv::processBeingChangeLookContinue(Net::MessageIn &msg,
                                               Being *const dstBeing,
                                               const uint8_t type,
                                               const int id,
                                               const int id2,
                                               const int *cards A_UNUSED)
{
    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setOtherTime();

    const ItemColor itemColor = ItemColorManager::getColorFromCards(cards);
    const std::string color;
    switch (type)
    {
        case 0:  // change race
            dstBeing->setSubtype(fromInt(id, BeingTypeId),
                dstBeing->getLook());
            break;
        case 1:  // eAthena LOOK_HAIR
            dstBeing->setHairColor(fromInt(id, ItemColor));
            dstBeing->setSpriteID(SPRITE_HAIR_COLOR, id * -1);
            break;
        case 2:  // LOOK_WEAPON Weapon ID in id, Shield ID in id2
            dstBeing->setSprite(SPRITE_BODY, id, "", itemColor, true);
            dstBeing->setSprite(SPRITE_FLOOR, id2);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_FLOOR);
            break;
        case 3:  // LOOK_HEAD_BOTTOM
            dstBeing->setSprite(SPRITE_WEAPON, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_WEAPON);
            break;
        case 4:  // LOOK_HEAD_TOP Change upper headgear for eAthena, hat for us
            dstBeing->setSprite(SPRITE_CLOTHES_COLOR, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_CLOTHES_COLOR);
            break;
        case 5:  // LOOK_HEAD_MID Change middle headgear for eathena,
                 // armor for us
            dstBeing->setSprite(SPRITE_HEAD_BOTTOM, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HEAD_BOTTOM);
            break;
        case 6:  // eAthena LOOK_HAIR_COLOR
            dstBeing->setHairColor(fromInt(id, ItemColor));
            dstBeing->setSpriteColor(SPRITE_HAIR_COLOR,
                ItemDB::get(dstBeing->getSpriteID(
                SPRITE_HAIR_COLOR)).getDyeColorsString(
                fromInt(id, ItemColor)));
            break;
        case 7:  // Clothes color. Now used as look
            dstBeing->setLook(static_cast<uint8_t>(id));
            break;
        case 8:  // eAthena LOOK_SHIELD
            dstBeing->setSprite(SPRITE_FLOOR, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_FLOOR);
            break;
        case 9:  // eAthena LOOK_SHOES
            dstBeing->setSprite(SPRITE_HAIR, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HAIR);
            break;
        case 10:  // LOOK_GLOVES
            dstBeing->setSprite(SPRITE_SHOES, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_SHOES);
            break;
        case 11:  // LOOK_FLOOR
            dstBeing->setSprite(SPRITE_SHIELD, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_SHIELD);
            break;
        case 12:  // LOOK_ROBE
            dstBeing->setSprite(SPRITE_HEAD_TOP, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HEAD_TOP);
            break;
        case 13:  // COSTUME_HEAD_TOP
            dstBeing->setSprite(SPRITE_HEAD_MID, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_HEAD_MID);
            break;
        case 14:  // COSTUME_HEAD_MID
            dstBeing->setSprite(SPRITE_ROBE, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_ROBE);
            break;
        case 15:  // COSTUME_HEAD_LOW
            dstBeing->setSprite(SPRITE_EVOL2, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL2);
            break;
        case 16:  // COSTUME_GARMENT
            dstBeing->setSprite(SPRITE_EVOL3, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL3);
            break;
        case 17:  // ARMOR
            dstBeing->setSprite(SPRITE_EVOL4, id, color, itemColor);
            if (localPlayer)
                localPlayer->imitateOutfit(dstBeing, SPRITE_EVOL4);
            break;
        default:
            UNIMPLIMENTEDPACKET;
            break;
    }
}

void BeingRecv::processBeingVisible(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    msg.readInt16("len");
    const BeingType::BeingType type = static_cast<BeingType::BeingType>(
        msg.readUInt8("object type"));

    // Information about a being in range
    const BeingId id = msg.readBeingId("being id");
    BeingId spawnId;
    if (id == Ea::BeingRecv::mSpawnId)
        spawnId = Ea::BeingRecv::mSpawnId;
    else
        spawnId = BeingId_zero;
    Ea::BeingRecv::mSpawnId = BeingId_zero;

    int16_t speed = msg.readInt16("speed");
    const uint16_t stunMode = msg.readInt16("opt1");
    // probably wrong effect usage
    const uint32_t statusEffects = msg.readInt16("opt2");
    msg.readInt32("option");

    const int16_t job = msg.readInt16("class");

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::Monster
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing2(msg, id, job, type);
        if (!dstBeing)
            return;
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::Npc)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();

    if (spawnId != BeingId_zero)
    {
        dstBeing->setAction(BeingAction::SPAWN, 0);
    }
    else
    {
        dstBeing->clearPath();
        dstBeing->setActionTime(tick_time);
        dstBeing->setAction(BeingAction::STAND, 0);
    }

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(Vector(speed, speed, 0));
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    if (dstBeing->getType() == ActorType::Monster && localPlayer)
        localPlayer->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16("hair style");
    const uint32_t weapon = static_cast<uint32_t>(msg.readInt32("weapon"));
    const uint16_t headBottom = msg.readInt16("head bottom");

    const uint16_t headTop = msg.readInt16("head top");
    const uint16_t headMid = msg.readInt16("head mid");
    const ItemColor hairColor = fromInt(msg.readInt16("hair color"), ItemColor);
    const uint16_t shoes = msg.readInt16("shoes or clothes color?");

    const uint16_t gloves = msg.readInt16("head dir / gloves");
    // may be use robe as gloves?
    msg.readInt16("robe");
    msg.readInt32("guild id");
    msg.readInt16("guild emblem");
    dstBeing->setManner(msg.readInt16("manner"));
    dstBeing->setStatusEffectBlock(32, static_cast<uint16_t>(
        msg.readInt32("opt3")));
    dstBeing->setKarma(msg.readUInt8("karma"));
    uint8_t gender = static_cast<uint8_t>(msg.readUInt8("gender") & 3);

    if (dstBeing->getType() == ActorType::Player)
    {
        dstBeing->setGender(Being::intToGender(gender));
        dstBeing->setHairColor(hairColor);
        // Set these after the gender, as the sprites may be gender-specific
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HAIR_COLOR, hairStyle * -1,
            ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_WEAPON, headBottom);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HEAD_BOTTOM, headMid);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_CLOTHES_COLOR, headTop);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HAIR, shoes);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_SHOES, gloves);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_BODY, weapon, "",
            ItemColor_one, true);
//        Ea::BeingRecv::setSprite(dstBeing, SPRITE_FLOOR, shield);
    }
    else if (dstBeing->getType() == ActorType::Npc
             && serverFeatures->haveNpcGender())
    {
        dstBeing->setGender(Being::intToGender(gender));
    }

    uint8_t dir;
    uint16_t x, y;
    msg.readCoordinates(x, y, dir, "position");
    msg.readInt8("xs");
    msg.readInt8("ys");
    msg.readUInt8("action type");
    dstBeing->setTileCoords(x, y);

    if (job == 45 && socialWindow && outfitWindow)
    {
        const int num = socialWindow->getPortalIndex(x, y);
        if (num >= 0)
        {
            dstBeing->setName(keyboard.getKeyShortString(
                outfitWindow->keyName(num)));
        }
        else
        {
            dstBeing->setName("");
        }
    }

    dstBeing->setDirection(dir);

    const int level = static_cast<int>(msg.readInt16("level"));
    if (level)
        dstBeing->setLevel(level);
    msg.readInt16("font");

    const int maxHP = msg.readInt32("max hp");
    const int hp = msg.readInt32("hp");
    dstBeing->setMaxHP(maxHP);
    dstBeing->setHP(hp);

    msg.readInt8("is boss");

    dstBeing->setStunMode(stunMode);
    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffffU));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffffU));
}

void BeingRecv::processBeingMove(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    msg.readInt16("len");
    const BeingType::BeingType type = static_cast<BeingType::BeingType>(
        msg.readUInt8("object type"));

    // Information about a being in range
    const BeingId id = msg.readBeingId("being id");
    BeingId spawnId;
    if (id == Ea::BeingRecv::mSpawnId)
        spawnId = Ea::BeingRecv::mSpawnId;
    else
        spawnId = BeingId_zero;
    Ea::BeingRecv::mSpawnId = BeingId_zero;
    int16_t speed = msg.readInt16("speed");
//    if (visible)
//    {
        const uint16_t stunMode = msg.readInt16("opt1");
        // probably wrong effect usage
        const uint32_t statusEffects = msg.readInt16("opt2");
//    }
//    else
//    {
// commented for now, probably it can be removed after testing
//        msg.readInt16("body state");
//        msg.readInt16("health state");
//    }
    msg.readInt32("effect state");

    const int16_t job = msg.readInt16("class");

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::Monster
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing2(msg, id, job, type);
        if (!dstBeing)
            return;
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::Npc)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();

    if (spawnId != BeingId_zero)
        dstBeing->setAction(BeingAction::SPAWN, 0);

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(Vector(speed, speed, 0));
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    if (dstBeing->getType() == ActorType::Monster && localPlayer)
        localPlayer->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16("hair style");
    const uint32_t weapon = static_cast<uint32_t>(msg.readInt32("weapon"));
    const uint16_t headBottom = msg.readInt16("head bottom");

    msg.readInt32("tick");

    const uint16_t headTop = msg.readInt16("head top");
    const uint16_t headMid = msg.readInt16("head mid");
    const ItemColor hairColor = fromInt(
        msg.readInt16("hair color"), ItemColor);
    const uint16_t shoes = msg.readInt16("shoes or clothes color?");

    const uint16_t gloves = msg.readInt16("head dir / gloves");
    // may be use robe as gloves?
    msg.readInt16("robe");
    msg.readInt32("guild id");
    msg.readInt16("guild emblem");
    dstBeing->setManner(msg.readInt16("manner"));
    dstBeing->setStatusEffectBlock(32, static_cast<uint16_t>(
        msg.readInt32("opt3")));
    dstBeing->setKarma(msg.readUInt8("karma"));
    uint8_t gender = static_cast<uint8_t>(msg.readUInt8("gender") & 3);

    if (dstBeing->getType() == ActorType::Player)
    {
        dstBeing->setGender(Being::intToGender(gender));
        dstBeing->setHairColor(hairColor);
        // Set these after the gender, as the sprites may be gender-specific
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HAIR_COLOR, hairStyle * -1,
            ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        if (!serverFeatures->haveMove3())
        {
            Ea::BeingRecv::setSprite(dstBeing, SPRITE_WEAPON, headBottom);
            Ea::BeingRecv::setSprite(dstBeing, SPRITE_HEAD_BOTTOM, headMid);
            Ea::BeingRecv::setSprite(dstBeing, SPRITE_CLOTHES_COLOR, headTop);
            Ea::BeingRecv::setSprite(dstBeing, SPRITE_HAIR, shoes);
            Ea::BeingRecv::setSprite(dstBeing, SPRITE_SHOES, gloves);
            Ea::BeingRecv::setSprite(dstBeing, SPRITE_BODY, weapon, "",
                ItemColor_one, true);
        }
//        Ea::BeingRecv::setSprite(dstBeing, SPRITE_FLOOR, shield);
    }
    else if (dstBeing->getType() == ActorType::Npc
             && serverFeatures->haveNpcGender())
    {
        dstBeing->setGender(Being::intToGender(gender));
    }

    uint16_t srcX, srcY, dstX, dstY;
    msg.readCoordinatePair(srcX, srcY, dstX, dstY, "move path");
    msg.readUInt8("(sx<<4) | (sy&0x0f)");
    msg.readInt8("xs");
    msg.readInt8("ys");
    dstBeing->setAction(BeingAction::STAND, 0);
    dstBeing->setTileCoords(srcX, srcY);
    if (localPlayer)
        localPlayer->followMoveTo(dstBeing, srcX, srcY, dstX, dstY);
    if (!serverFeatures->haveMove3())
        dstBeing->setDestination(dstX, dstY);

    // because server don't send direction in move packet, we fixing it

    uint8_t d = 0;
    if (srcX == dstX && srcY == dstY)
    {   // if player did one step from invisible area to visible,
        // move path is broken
        int x2 = localPlayer->getTileX();
        int y2 = localPlayer->getTileY();
        if (abs(x2 - srcX) > abs(y2 - srcY))
            y2 = srcY;
        else
            x2 = srcX;
        d = dstBeing->calcDirection(x2, y2);
    }
    else
    {
        d = dstBeing->calcDirection(dstX, dstY);
    }
    if (d && dstBeing->getDirection() != d)
        dstBeing->setDirection(d);

    const int level = static_cast<int>(msg.readInt16("level"));
    if (level)
        dstBeing->setLevel(level);
    msg.readInt16("font");

    const int maxHP = msg.readInt32("max hp");
    const int hp = msg.readInt32("hp");
    dstBeing->setMaxHP(maxHP);
    dstBeing->setHP(hp);

    msg.readInt8("is boss");

    dstBeing->setStunMode(stunMode);
    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffffU));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffffU));
}

void BeingRecv::processBeingSpawn(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    msg.readInt16("len");
    const BeingType::BeingType type = static_cast<BeingType::BeingType>(
        msg.readUInt8("object type"));

    // Information about a being in range
    const BeingId id = msg.readBeingId("being id");
    Ea::BeingRecv::mSpawnId = id;
    const BeingId spawnId = id;
    int16_t speed = msg.readInt16("speed");
//    if (visible)
//    {
        const uint16_t stunMode = msg.readInt16("opt1");
        // probably wrong effect usage
        const uint32_t statusEffects = msg.readInt16("opt2");
//    }
//    else
//    {
// commented for now, probably it can be removed after testing
//        msg.readInt16("body state");
//        msg.readInt16("health state");
//    }
    msg.readInt32("effect state");

    const int16_t job = msg.readInt16("class");

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::Monster
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing2(msg, id, job, type);
        if (!dstBeing)
            return;
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::Npc)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();

    if (spawnId != BeingId_zero)
        dstBeing->setAction(BeingAction::SPAWN, 0);

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(Vector(speed, speed, 0));
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    if (dstBeing->getType() == ActorType::Monster && localPlayer)
        localPlayer->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16("hair style");
    const uint32_t weapon = static_cast<uint32_t>(msg.readInt32("weapon"));
    const uint16_t headBottom = msg.readInt16("head bottom");

    const uint16_t headTop = msg.readInt16("head top");
    const uint16_t headMid = msg.readInt16("head mid");
    const ItemColor hairColor = fromInt(
        msg.readInt16("hair color"), ItemColor);
    const uint16_t shoes = msg.readInt16("shoes or clothes color?");
    const uint16_t gloves = msg.readInt16("head dir / gloves");
    // may be use robe as gloves?
    msg.readInt16("robe");
    msg.readInt32("guild id");
    msg.readInt16("guild emblem");
    dstBeing->setManner(msg.readInt16("manner"));
    dstBeing->setStatusEffectBlock(32, static_cast<uint16_t>(
        msg.readInt32("opt3")));
    dstBeing->setKarma(msg.readUInt8("karma"));
    uint8_t gender = static_cast<uint8_t>(msg.readUInt8("gender") & 3);

    if (dstBeing->getType() == ActorType::Player)
    {
        dstBeing->setGender(Being::intToGender(gender));
        dstBeing->setHairColor(hairColor);
        // Set these after the gender, as the sprites may be gender-specific
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HAIR_COLOR, hairStyle * -1,
            ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_WEAPON, headBottom);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HEAD_BOTTOM, headMid);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_CLOTHES_COLOR, headTop);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_HAIR, shoes);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_SHOES, gloves);
        Ea::BeingRecv::setSprite(dstBeing, SPRITE_BODY, weapon, "",
            ItemColor_one, true);
//        Ea::BeingRecv::setSprite(dstBeing, SPRITE_FLOOR, shield);
    }
    else if (dstBeing->getType() == ActorType::Npc
             && serverFeatures->haveNpcGender())
    {
        dstBeing->setGender(Being::intToGender(gender));
    }

    uint8_t dir;
    uint16_t x, y;
    msg.readCoordinates(x, y, dir, "position");
    msg.readInt8("xs");
    msg.readInt8("ys");
    dstBeing->setTileCoords(x, y);

    if (job == 45 && socialWindow && outfitWindow)
    {
        const int num = socialWindow->getPortalIndex(x, y);
        if (num >= 0)
        {
            dstBeing->setName(keyboard.getKeyShortString(
                outfitWindow->keyName(num)));
        }
        else
        {
            dstBeing->setName("");
        }
    }

    dstBeing->setDirection(dir);

    const int level = static_cast<int>(msg.readInt16("level"));
    if (level)
        dstBeing->setLevel(level);
    msg.readInt16("font");

    const int maxHP = msg.readInt32("max hp");
    const int hp = msg.readInt32("hp");
    dstBeing->setMaxHP(maxHP);
    dstBeing->setHP(hp);

    msg.readInt8("is boss");

    dstBeing->setStunMode(stunMode);
    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffffU));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffffU));
}

void BeingRecv::processMapTypeProperty(Net::MessageIn &msg)
{
    const int16_t type = msg.readInt16("type");
    const int flags = msg.readInt32("flags");
    if (type == 0x28)
    {
        // +++ need get other flags from here
        MapTypeProperty2 props;
        props.data = static_cast<uint32_t>(flags);
        Game *const game = Game::instance();
        if (!game)
            return;
        Map *const map = game->getCurrentMap();
        if (!map)
            return;
        map->setPvpMode(props.bits.party | (props.bits.guild * 2));
    }
}

void BeingRecv::processMapType(Net::MessageIn &msg)
{
    const int16_t type = msg.readInt16("type");
    if (type == 19)
        NotifyManager::notify(NotifyTypes::MAP_TYPE_BATTLEFIELD);
    else
        UNIMPLIMENTEDPACKET;
}

void BeingRecv::processSkillCasting(Net::MessageIn &msg)
{
    // +++ need use other parameters

    const BeingId srcId = msg.readBeingId("src id");
    const BeingId dstId = msg.readBeingId("dst id");
    const int dstX = msg.readInt16("dst x");
    const int dstY = msg.readInt16("dst y");
    const int skillId = msg.readInt16("skill id");
    msg.readInt32("property");  // can be used to trigger effect
    msg.readInt32("cast time");
    msg.readInt8("dispossable");

    if (!effectManager)
        return;

    if (srcId == BeingId_zero)
    {
        UNIMPLIMENTEDPACKET;
        return;
    }
    else if (dstId != BeingId_zero)
    {   // being to being
        Being *const srcBeing = actorManager->findBeing(srcId);
        Being *const dstBeing = actorManager->findBeing(dstId);
        skillDialog->playCastingSrcEffect(skillId, srcBeing);
        skillDialog->playCastingDstEffect(skillId, dstBeing);
    }
    else if (dstX != 0 || dstY != 0)
    {   // being to position
        UNIMPLIMENTEDPACKET;
    }
}

void BeingRecv::processBeingStatusChange(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingStatusChange")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingStatusChange")
        return;
    }

    // Status change
    const uint16_t status = msg.readInt16("status");
    const BeingId id = msg.readBeingId("being id");
    const Enable flag = fromBool(
        msg.readUInt8("flag: 0: stop, 1: start"), Enable);
    msg.readInt32("total");
    msg.readInt32("left");
    msg.readInt32("val1");
    msg.readInt32("val2");
    msg.readInt32("val3");

    Being *const dstBeing = actorManager->findBeing(id);
    if (dstBeing)
        dstBeing->setStatusEffect(status, flag);
    BLOCK_END("BeingRecv::processBeingStatusChange")
}

void BeingRecv::processBeingStatusChange2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingStatusChange")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingStatusChange")
        return;
    }

    // Status change
    const uint16_t status = msg.readInt16("status");
    const BeingId id = msg.readBeingId("being id");
    const Enable flag = fromBool(
        msg.readUInt8("flag: 0: stop, 1: start"), Enable);
    msg.readInt32("left");
    msg.readInt32("val1");
    msg.readInt32("val2");
    msg.readInt32("val3");

    Being *const dstBeing = actorManager->findBeing(id);
    if (dstBeing)
        dstBeing->setStatusEffect(status, flag);
    BLOCK_END("BeingRecv::processBeingStatusChange")
}

void BeingRecv::processBeingMove2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingMove2")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingMove2")
        return;
    }

    /*
      * A simplified movement packet, used by the
      * later versions of eAthena for both mobs and
      * players
      */
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));

    uint16_t srcX, srcY, dstX, dstY;
    msg.readCoordinatePair(srcX, srcY, dstX, dstY, "move path");
    msg.readUInt8("(sx<<4) | (sy&0x0f)");
    msg.readInt32("tick");

    /*
      * This packet doesn't have enough info to actually
      * create a new being, so if the being isn't found,
      * we'll just pretend the packet didn't happen
      */

    if (!dstBeing)
    {
        BLOCK_END("BeingRecv::processBeingMove2")
        return;
    }

    dstBeing->setTileCoords(srcX, srcY);
    if (localPlayer)
        localPlayer->followMoveTo(dstBeing, srcX, srcY, dstX, dstY);
    if (!serverFeatures->haveMove3())
        dstBeing->setDestination(dstX, dstY);
    if (dstBeing->getType() == ActorType::Player)
        dstBeing->setMoveTime();
    BLOCK_END("BeingRecv::processBeingMove2")
}

void BeingRecv::processBeingAction2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingAction2")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingAction2")
        return;
    }

    Being *const srcBeing = actorManager->findBeing(
        msg.readBeingId("src being id"));
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("dst being id"));

    msg.readInt32("tick");
    const int srcSpeed = msg.readInt32("src speed");
    msg.readInt32("dst speed");
    const int param1 = msg.readInt32("damage");
    msg.readInt16("count");
    const AttackTypeT type = static_cast<AttackTypeT>(
        msg.readUInt8("action"));
    msg.readInt32("left damage");

    switch (type)
    {
        case AttackType::HIT:  // Damage
        case AttackType::CRITICAL:  // Critical Damage
        case AttackType::MULTI:  // Critical Damage
        case AttackType::MULTI_REFLECT:
        case AttackType::REFLECT:  // Reflected Damage
        case AttackType::FLEE:  // Lucky Dodge
        case AttackType::SPLASH:
        case AttackType::SKILL:
        case AttackType::REPEATE:
            if (srcBeing)
            {
                if (srcSpeed && srcBeing->getType() == ActorType::Player)
                    srcBeing->setAttackDelay(srcSpeed);
                // attackid=1, type
                srcBeing->handleAttack(dstBeing, param1, 1);
                if (srcBeing->getType() == ActorType::Player)
                    srcBeing->setAttackTime();
            }
            if (dstBeing)
            {
                // level not present, using 1
                dstBeing->takeDamage(srcBeing, param1,
                    static_cast<AttackTypeT>(type), 1);
            }
            break;

        case AttackType::PICKUP:
            break;

        case AttackType::TOUCH_SKILL:
            break;

        case AttackType::SIT:
            if (srcBeing)
            {
                srcBeing->setAction(BeingAction::SIT, 0);
                if (srcBeing->getType() == ActorType::Player)
                {
                    srcBeing->setMoveTime();
                    if (localPlayer)
                        localPlayer->imitateAction(srcBeing, BeingAction::SIT);
                }
            }
            break;

        case AttackType::STAND:
            if (srcBeing)
            {
                srcBeing->setAction(BeingAction::STAND, 0);
                if (srcBeing->getType() == ActorType::Player)
                {
                    srcBeing->setMoveTime();
                    if (localPlayer)
                    {
                        localPlayer->imitateAction(srcBeing,
                            BeingAction::STAND);
                    }
                }
            }
            break;
        default:
        case AttackType::MISS:
        case AttackType::SKILLMISS:
            UNIMPLIMENTEDPACKET;
            break;
    }
    BLOCK_END("BeingRecv::processBeingAction2")
}

void BeingRecv::processMonsterHp(Net::MessageIn &msg)
{
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("monster id"));
    const int hp = msg.readInt32("hp");
    const int maxHP = msg.readInt32("max hp");
    if (dstBeing)
    {
        dstBeing->setHP(hp);
        dstBeing->setMaxHP(maxHP);
    }
}

void BeingRecv::processSkillAutoCast(Net::MessageIn &msg)
{
    const int id = msg.readInt16("skill id");
    msg.readInt16("inf");
    msg.readInt16("unused");
    const int level = msg.readInt16("skill level");
    msg.readInt16("sp");
    msg.readInt16("range");
    msg.readString(24, "skill name");
    msg.readInt8("unused");

    if (localPlayer)
    {
        localPlayer->handleSkill(localPlayer, 0, id, level);
        localPlayer->takeDamage(localPlayer, 0, AttackType::SKILL, id, level);
    }
}

void BeingRecv::processRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    msg.readInt16("rank type");
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
    msg.readInt32("my points");
}

void BeingRecv::processBlacksmithRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processAlchemistRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processTaekwonRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processPkRanksList(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ here need window with rank tables.
    for (int f = 0; f < 10; f ++)
        msg.readString(24, "name");
    for (int f = 0; f < 10; f ++)
        msg.readInt32("points");
}

void BeingRecv::processBeingChangeDirection(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingChangeDirection")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processBeingChangeDirection")
        return;
    }

    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));

    msg.readInt16("head direction");

    const uint8_t dir = Net::MessageIn::fromServerDirection(
        static_cast<uint8_t>(msg.readUInt8("player direction") & 0x0FU));

    if (!dstBeing)
    {
        BLOCK_END("BeingRecv::processBeingChangeDirection")
        return;
    }

    dstBeing->setDirection(dir);
    if (localPlayer)
        localPlayer->imitateDirection(dstBeing, dir);
    BLOCK_END("BeingRecv::processBeingChangeDirection")
}

void BeingRecv::processBeingSpecialEffect(Net::MessageIn &msg)
{
    if (!effectManager || !actorManager)
        return;

    const BeingId id = msg.readBeingId("being id");
    Being *const being = actorManager->findBeing(id);
    if (!being)
        return;

    const int effectType = msg.readInt32("effect type");

    if (Particle::enabled)
        effectManager->trigger(effectType, being);

    // +++ need dehard code effectType == 3
    if (effectType == 3 && being->getType() == ActorType::Player
        && socialWindow)
    {   // reset received damage
        socialWindow->resetDamage(being->getName());
    }
}

void BeingRecv::processBeingSpecialEffectNum(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ need somhow show this effects.
    // type is not same with self/misc effect.
    msg.readBeingId("account id");
    msg.readInt32("effect type");
    msg.readInt32("num");  // effect variable
}

void BeingRecv::processBeingSoundEffect(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ need play this effect.
    msg.readString(24, "sound effect name");
    msg.readUInt8("type");
    msg.readInt32("unused");
    msg.readInt32("source being id");
}

void BeingRecv::processSkillGroundNoDamage(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("skill id");
    msg.readInt32("src id");
    msg.readInt16("val");
    msg.readInt16("x");
    msg.readInt16("y");
    msg.readInt32("tick");
}

void BeingRecv::processSkillEntry(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    msg.readInt16("len");
    msg.readInt32("accound id");
    msg.readInt32("creator accound id");
    msg.readInt16("x");
    msg.readInt16("y");
    msg.readInt32("job");
    msg.readUInt8("radius");
    msg.readUInt8("visible");
    msg.readUInt8("level");
}

void BeingRecv::processPlaterStatusChange(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processPlayerStop")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processPlayerStop")
        return;
    }

    // Change in players' flags
    const BeingId id = msg.readBeingId("account id");
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    const uint16_t stunMode = msg.readInt16("stun mode");
    uint32_t statusEffects = msg.readInt16("status effect");
    statusEffects |= (static_cast<uint32_t>(msg.readInt32("opt?"))) << 16;
    dstBeing->setKarma(msg.readUInt8("karma"));

    dstBeing->setStunMode(stunMode);
    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffff));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffff));
    BLOCK_END("BeingRecv::processPlayerStop")
}

void BeingRecv::processPlaterStatusChange2(Net::MessageIn &msg)
{
    if (!actorManager)
        return;

    const BeingId id = msg.readBeingId("account id");
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    uint32_t statusEffects = msg.readInt32("status effect");
    dstBeing->setLevel(msg.readInt32("level"));
    msg.readInt32("showEFST");

    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffff));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffff));
}

void BeingRecv::processPlaterStatusChangeNoTick(Net::MessageIn &msg)
{
    const uint16_t status = msg.readInt16("index");
    const BeingId id = msg.readBeingId("account id");
    const Enable flag = fromBool(msg.readUInt8("state")
        ? true : false, Enable);

    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    dstBeing->setStatusEffect(status, flag);
}

void BeingRecv::processBeingResurrect(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingResurrect")
    if (!actorManager || !localPlayer)
    {
        BLOCK_END("BeingRecv::processBeingResurrect")
        return;
    }

    // A being changed mortality status

    const BeingId id = msg.readBeingId("being id");
    msg.readInt16("unused");
    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
    {
        BLOCK_END("BeingRecv::processBeingResurrect")
        return;
    }

    // If this is player's current target, clear it.
    if (dstBeing == localPlayer->getTarget())
        localPlayer->stopAttack();

    dstBeing->setAction(BeingAction::STAND, 0);
    BLOCK_END("BeingRecv::processBeingResurrect")
}

void BeingRecv::processPlayerGuilPartyInfo(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processPlayerGuilPartyInfo")
    if (!actorManager)
    {
        BLOCK_END("BeingRecv::processPlayerGuilPartyInfo")
        return;
    }

    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("being id"));

    if (dstBeing)
    {
        dstBeing->setName(msg.readString(24, "char name"));
        dstBeing->setPartyName(msg.readString(24, "party name"));
        dstBeing->setGuildName(msg.readString(24, "guild name"));
        dstBeing->setGuildPos(msg.readString(24, "guild pos"));
        dstBeing->addToCache();
    }
    BLOCK_END("BeingRecv::processPlayerGuilPartyInfo")
}

void BeingRecv::processBeingRemoveSkil(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;
    // +++ if skill unit was added, here need remove it from actors
    msg.readInt32("skill unit id");
}

void BeingRecv::processBeingFakeName(Net::MessageIn &msg)
{
    const BeingType::BeingType type = static_cast<BeingType::BeingType>(
        msg.readUInt8("object type"));
    const BeingId id = msg.readBeingId("npc id");
    msg.skip(8, "unused");
    const uint16_t job = msg.readInt16("class?");  // 111
    msg.skip(30, "unused");
    uint16_t x, y;
    uint8_t dir;
    msg.readCoordinates(x, y, dir, "position");
    msg.readUInt8("sx");
    msg.readUInt8("sy");
    msg.skip(4, "unsued");

    Being *const dstBeing = createBeing2(msg, id, job, type);
    if (!dstBeing)
        return;
    dstBeing->setSubtype(fromInt(job, BeingTypeId), 0);
    dstBeing->setTileCoords(x, y);
    dstBeing->setDirection(dir);
}

void BeingRecv::processBeingStatUpdate1(Net::MessageIn &msg)
{
    const BeingId id = msg.readBeingId("account id");
    const int type = msg.readInt16("type");
    const int value = msg.readInt32("value");

    Being *const dstBeing = actorManager->findBeing(id);
    if (!dstBeing)
        return;

    if (type != Ea::MANNER)
    {
        UNIMPLIMENTEDPACKET;
        return;
    }
    dstBeing->setManner(value);
}

void BeingRecv::processBeingSelfEffect(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processBeingSelfEffect")
    if (!effectManager || !actorManager)
    {
        BLOCK_END("BeingRecv::processBeingSelfEffect")
        return;
    }

    const BeingId id = msg.readBeingId("being id");
    Being *const being = actorManager->findBeing(id);
    if (!being)
    {
        BLOCK_END("BeingRecv::processBeingSelfEffect")
        return;
    }

    const int effectType = msg.readInt32("effect type");
    if (Particle::enabled)
        effectManager->trigger(effectType, being);

    BLOCK_END("BeingRecv::processBeingSelfEffect")
}

void BeingRecv::processMobInfo(Net::MessageIn &msg)
{
    const int len = msg.readInt16("len");
    if (len < 12)
        return;
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("monster id"));
    const int attackRange = msg.readInt32("range");
    if (dstBeing)
        dstBeing->setAttackRange(attackRange);
}

void BeingRecv::processBeingAttrs(Net::MessageIn &msg)
{
    const int len = msg.readInt16("len");
    if (len < 12)
        return;
    Being *const dstBeing = actorManager->findBeing(
        msg.readBeingId("player id"));
    const int gmLevel = msg.readInt32("gm level");
    if (dstBeing && gmLevel)
    {
        if (dstBeing == localPlayer)
            localPlayer->setGMLevel(gmLevel);
        dstBeing->setGM(true);
    }
    else
    {
        if (dstBeing == localPlayer)
            localPlayer->setGMLevel(0);
        if (dstBeing)
            dstBeing->setGM(false);
    }
}

void BeingRecv::processMonsterInfo(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readInt16("class");
    msg.readInt16("level");
    msg.readInt16("size");
    msg.readInt32("hp");
    msg.readInt16("def");
    msg.readInt16("race");
    msg.readInt16("mdef");
    msg.readInt16("ele");
}

void BeingRecv::processClassChange(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readUInt8("type");
    msg.readInt32("class");
}

void BeingRecv::processSpiritBalls(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readInt16("spirits amount");
}

void BeingRecv::processSpiritBallSingle(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readInt16("spirits amount");
}

void BeingRecv::processBladeStop(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readInt32("src being id");
    msg.readInt32("dst being id");
    msg.readInt32("flag");
}

void BeingRecv::processComboDelay(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readInt32("wait");
}

void BeingRecv::processWddingEffect(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
}

void BeingRecv::processBeingSlide(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("being id");
    msg.readInt16("x");
    msg.readInt16("y");
}

void BeingRecv::processStarsKill(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readString(24, "map name");
    msg.readInt32("monster id");
    msg.readUInt8("start");
    msg.readUInt8("result");
}

void BeingRecv::processGladiatorFeelRequest(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readUInt8("which");
}

void BeingRecv::processBossMapInfo(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readUInt8("info type");
    msg.readInt32("x");
    msg.readInt32("y");
    msg.readInt16("min hours");
    msg.readInt16("min minutes");
    msg.readInt16("max hours");
    msg.readInt16("max minutes");
    msg.readString(24, "monster name");  // really can be used 51 byte?
}

void BeingRecv::processBeingFont(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("account id");
    msg.readInt16("font");
}

void BeingRecv::processBeingMilleniumShield(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("account id");
    msg.readInt16("shields");
    msg.readInt16("unused");
}

void BeingRecv::processBeingCharm(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    msg.readBeingId("account id");
    msg.readInt16("charm type");
    msg.readInt16("charm count");
}

void BeingRecv::processBeingViewEquipment(Net::MessageIn &msg)
{
    UNIMPLIMENTEDPACKET;

    const int count = (msg.readInt16("len") - 45) / 31;
    msg.readString(24, "name");
    msg.readInt16("job");
    msg.readInt16("head");
    msg.readInt16("accessory");
    msg.readInt16("accessory2");
    msg.readInt16("accessory3");
    msg.readInt16("robe");
    msg.readInt16("hair color");
    msg.readInt16("body color");
    msg.readUInt8("gender");
    for (int f = 0; f < count; f ++)
    {
        msg.readInt16("index");
        msg.readInt16("item id");
        msg.readUInt8("item type");
        msg.readInt32("location");
        msg.readInt32("wear state");
        msg.readInt8("refine");
        for (int d = 0; d < 4; d ++)
            msg.readInt16("card");
        msg.readInt32("hire expire date (?)");
        msg.readInt16("equip type");
        msg.readInt16("item sprite number");
        msg.readUInt8("flags");
    }
}

void BeingRecv::processPvpSet(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processPvpSet")
    const BeingId id = msg.readBeingId("being id");
    const int rank = msg.readInt32("rank");
    msg.readInt32("num");
    if (actorManager)
    {
        Being *const dstBeing = actorManager->findBeing(id);
        if (dstBeing)
            dstBeing->setPvpRank(rank);
    }
    BLOCK_END("BeingRecv::processPvpSet")
}

void BeingRecv::processNameResponse2(Net::MessageIn &msg)
{
    BLOCK_START("BeingRecv::processNameResponse2")
    if (!actorManager || !localPlayer)
    {
        BLOCK_END("BeingRecv::processNameResponse2")
        return;
    }

    const int len = msg.readInt16("len");
    const BeingId beingId = msg.readBeingId("account ic");
    const std::string str = msg.readString(len - 8, "name");
    Being *const dstBeing = actorManager->findBeing(beingId);
    if (dstBeing)
    {
        if (beingId == localPlayer->getId())
        {
            localPlayer->pingResponse();
        }
        else
        {
            dstBeing->setName(str);
            dstBeing->updateGuild();
            dstBeing->addToCache();

            if (dstBeing->getType() == ActorType::Player)
                dstBeing->updateColors();

            if (localPlayer)
            {
                const Party *const party = localPlayer->getParty();
                if (party && party->isMember(dstBeing->getId()))
                {
                    PartyMember *const member = party->getMember(
                        dstBeing->getId());

                    if (member)
                        member->setName(dstBeing->getName());
                }
                localPlayer->checkNewName(dstBeing);
            }
        }
    }
    BLOCK_END("BeingRecv::processNameResponse2")
}

Being *BeingRecv::createBeing2(Net::MessageIn &msg,
                               const BeingId id,
                               const int16_t job,
                               const BeingType::BeingType beingType)
{
    if (!actorManager)
        return nullptr;

    ActorTypeT type = ActorType::Unknown;
    switch (beingType)
    {
        case BeingType::PC:
            type = ActorType::Player;
            break;
        case BeingType::NPC:
        case BeingType::NPC_EVENT:
            type = ActorType::Npc;
            break;
        case BeingType::MONSTER:
            type = ActorType::Monster;
            break;
        case BeingType::MERSOL:
            type = ActorType::Mercenary;
            break;
        case BeingType::PET:
            type = ActorType::Pet;
            break;
        case BeingType::HOMUN:
            type = ActorType::Homunculus;
            break;
        case BeingType::ITEM:
        case BeingType::SKILL:
        case BeingType::ELEMENTAL:
            logger->log("not supported object type: %d, job: %d",
                static_cast<int>(beingType), static_cast<int>(job));
            break;
        case BeingType::CHAT:
        default:
            UNIMPLIMENTEDPACKET;
            type = ActorType::Monster;
            logger->log("not supported object type: %d, job: %d",
                static_cast<int>(beingType), static_cast<int>(job));
            break;
    }
    if (job == 45 && beingType == BeingType::NPC_EVENT)
        type = ActorType::Portal;

    Being *const being = actorManager->createBeing(
        id, type, fromInt(job, BeingTypeId));
    if (beingType == BeingType::MERSOL)
    {
        MercenaryInfo *const info = PlayerInfo::getMercenary();
        if (info && info->id == id)
            PlayerInfo::setMercenaryBeing(being);
    }
    else if (beingType == BeingType::PET)
    {
        if (PlayerInfo::getPetBeingId() == id)
            PlayerInfo::setPetBeing(being);
    }
    return being;
}

}  // namespace EAthena