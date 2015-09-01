/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "net/eathena/generalhandler.h"

#include "client.h"
#include "configuration.h"

#include "gui/windows/skilldialog.h"
#include "gui/windows/socialwindow.h"
#include "gui/windows/statuswindow.h"

#include "gui/widgets/tabs/chat/guildtab.h"
#include "gui/widgets/tabs/chat/partytab.h"

#include "net/eathena/generalrecv.h"

#include "net/eathena/adminhandler.h"
#include "net/eathena/auctionhandler.h"
#include "net/eathena/bankhandler.h"
#include "net/eathena/beinghandler.h"
#include "net/eathena/buyingstorehandler.h"
#include "net/eathena/buysellhandler.h"
#include "net/eathena/cashshophandler.h"
#include "net/eathena/chathandler.h"
#include "net/eathena/charserverhandler.h"
#include "net/eathena/elementalhandler.h"
#include "net/eathena/familyhandler.h"
#include "net/eathena/friendshandler.h"
#include "net/eathena/gamehandler.h"
#include "net/eathena/guildhandler.h"
#include "net/eathena/homunculushandler.h"
#include "net/eathena/inventoryhandler.h"
#include "net/eathena/itemhandler.h"
#include "net/eathena/loginhandler.h"
#include "net/eathena/mailhandler.h"
#include "net/eathena/markethandler.h"
#include "net/eathena/mercenaryhandler.h"
#include "net/eathena/network.h"
#include "net/eathena/npchandler.h"
#include "net/eathena/partyhandler.h"
#include "net/eathena/pethandler.h"
#include "net/eathena/playerhandler.h"
#include "net/eathena/protocol.h"
#include "net/eathena/roulettehandler.h"
#include "net/eathena/searchstorehandler.h"
#include "net/eathena/serverfeatures.h"
#include "net/eathena/tradehandler.h"
#include "net/eathena/skillhandler.h"
#include "net/eathena/questhandler.h"
#include "net/eathena/vendinghandler.h"

#include "resources/db/itemdbstat.h"

#include "utils/delete2.h"
#include "utils/gettext.h"

#include "debug.h"

extern Net::GeneralHandler *generalHandler;

namespace EAthena
{

GeneralHandler::GeneralHandler() :
    MessageHandler(),
    mAdminHandler(new AdminHandler),
    mBeingHandler(new BeingHandler(config.getBoolValue("EnableSync"))),
    mBuySellHandler(new BuySellHandler),
    mCharServerHandler(new CharServerHandler),
    mChatHandler(new ChatHandler),
    mGameHandler(new GameHandler),
    mGuildHandler(new GuildHandler),
    mInventoryHandler(new InventoryHandler),
    mItemHandler(new ItemHandler),
    mLoginHandler(new LoginHandler),
    mNpcHandler(new NpcHandler),
    mPartyHandler(new PartyHandler),
    mPetHandler(new PetHandler),
    mPlayerHandler(new PlayerHandler),
    mSkillHandler(new SkillHandler),
    mTradeHandler(new TradeHandler),
    mQuestHandler(new QuestHandler),
    mServerFeatures(new ServerFeatures),
    mMailHandler(new MailHandler),
    mAuctionHandler(new AuctionHandler),
    mCashShopHandler(new CashShopHandler),
    mFamilyHandler(new FamilyHandler),
    mBankHandler(new BankHandler),
    mBattleGroundHandler(new BankHandler),
    mMercenaryHandler(new MercenaryHandler),
    mBuyingStoreHandler(new BuyingStoreHandler),
    mHomunculusHandler(new HomunculusHandler),
    mFriendsHandler(new FriendsHandler),
    mElementalHandler(new ElementalHandler),
    mMapHandler(new MarketHandler),
    mMarketHandler(new MarketHandler),
    mVendingHandler(new VendingHandler),
    mRouletteHandler(new RouletteHandler),
    mSearchStoreHandler(new SearchStoreHandler)
{
    static const uint16_t _messages[] =
    {
        SMSG_CONNECTION_PROBLEM,
        SMSG_MAP_NOT_FOUND,
        0
    };
    handledMessages = _messages;
    generalHandler = this;

    std::vector<ItemDB::Stat> stats;
    stats.push_back(ItemDB::Stat("str", _("Strength %s")));
    stats.push_back(ItemDB::Stat("agi", _("Agility %s")));
    stats.push_back(ItemDB::Stat("vit", _("Vitality %s")));
    stats.push_back(ItemDB::Stat("int", _("Intelligence %s")));
    stats.push_back(ItemDB::Stat("dex", _("Dexterity %s")));
    stats.push_back(ItemDB::Stat("luck", _("Luck %s")));

    ItemDB::setStatsList(stats);
}

GeneralHandler::~GeneralHandler()
{
    delete2(mNetwork);
}

void GeneralHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_CONNECTION_PROBLEM:
            GeneralRecv::processConnectionProblem(msg);
            break;

        case SMSG_MAP_NOT_FOUND:
            GeneralRecv::processMapNotFound(msg);
            break;

        default:
            break;
    }
}

void GeneralHandler::load()
{
    mNetwork = new Network;
    mNetwork->registerHandlers();

    mNetwork->registerHandler(mAdminHandler);
    mNetwork->registerHandler(mBeingHandler);
    mNetwork->registerHandler(mBuySellHandler);
    mNetwork->registerHandler(mChatHandler);
    mNetwork->registerHandler(mCharServerHandler);
    mNetwork->registerHandler(mGameHandler);
    mNetwork->registerHandler(mGuildHandler);
    mNetwork->registerHandler(mInventoryHandler);
    mNetwork->registerHandler(mItemHandler);
    mNetwork->registerHandler(mLoginHandler);
    mNetwork->registerHandler(mNpcHandler);
    mNetwork->registerHandler(mPlayerHandler);
    mNetwork->registerHandler(mSkillHandler);
    mNetwork->registerHandler(mTradeHandler);
    mNetwork->registerHandler(mPartyHandler);
    mNetwork->registerHandler(mPetHandler);
    mNetwork->registerHandler(mQuestHandler);
    mNetwork->registerHandler(mMailHandler);
    mNetwork->registerHandler(mAuctionHandler);
    mNetwork->registerHandler(mCashShopHandler);
    mNetwork->registerHandler(mFamilyHandler);
    mNetwork->registerHandler(mBankHandler);
    mNetwork->registerHandler(mBattleGroundHandler);
    mNetwork->registerHandler(mMercenaryHandler);
    mNetwork->registerHandler(mBuyingStoreHandler);
    mNetwork->registerHandler(mHomunculusHandler);
    mNetwork->registerHandler(mFriendsHandler);
    mNetwork->registerHandler(mElementalHandler);
    mNetwork->registerHandler(mMapHandler);
    mNetwork->registerHandler(mMarketHandler);
    mNetwork->registerHandler(mVendingHandler);
    mNetwork->registerHandler(mSearchStoreHandler);
    mNetwork->registerHandler(mRouletteHandler);
}

void GeneralHandler::reload()
{
    if (mNetwork)
        mNetwork->disconnect();

    static_cast<LoginHandler*>(mLoginHandler)->clearWorlds();
    CharServerHandler *const charHandler = static_cast<CharServerHandler*>(
        mCharServerHandler);
    charHandler->setCharCreateDialog(nullptr);
    charHandler->setCharSelectDialog(nullptr);
    static_cast<PartyHandler*>(mPartyHandler)->reload();
}

void GeneralHandler::reloadPartially() const
{
    static_cast<PartyHandler*>(mPartyHandler)->reload();
}

void GeneralHandler::unload()
{
    clearHandlers();
}

void GeneralHandler::flushSend()
{
    if (!mNetwork)
        return;

    mNetwork->flush();
}

void GeneralHandler::flushNetwork()
{
    if (!mNetwork)
        return;

    mNetwork->flush();
    mNetwork->dispatchMessages();

    if (mNetwork->getState() == Network::NET_ERROR)
    {
        if (!mNetwork->getError().empty())
            errorMessage = mNetwork->getError();
        else
            errorMessage = _("Got disconnected from server!");

        client->setState(STATE_ERROR);
    }
}

void GeneralHandler::clearHandlers()
{
    if (mNetwork)
        mNetwork->clearHandlers();
}

void GeneralHandler::gameStarted() const
{
    if (skillDialog)
        skillDialog->loadSkills();

    if (!statusWindow)
        return;

    // protection against double addition attributes.
    statusWindow->clearAttributes();

    statusWindow->addAttribute(Attributes::STR,
        _("Strength"), "str", Modifiable_true);
    statusWindow->addAttribute(Attributes::AGI,
        _("Agility"), "agi", Modifiable_true);
    statusWindow->addAttribute(Attributes::VIT,
        _("Vitality"), "vit", Modifiable_true);
    statusWindow->addAttribute(Attributes::INT,
        _("Intelligence"), "int", Modifiable_true);
    statusWindow->addAttribute(Attributes::DEX,
        _("Dexterity"), "dex", Modifiable_true);
    statusWindow->addAttribute(Attributes::LUK,
        _("Luck"), "luk", Modifiable_true);

    statusWindow->addAttribute(Attributes::ATK, _("Attack"));
    statusWindow->addAttribute(Attributes::DEF, _("Defense"));
    statusWindow->addAttribute(Attributes::MATK, _("M.Attack"));
    statusWindow->addAttribute(Attributes::MDEF, _("M.Defense"));
    // xgettext:no-c-format
    statusWindow->addAttribute(Attributes::HIT, _("% Accuracy"));
    // xgettext:no-c-format
    statusWindow->addAttribute(Attributes::FLEE, _("% Evade"));
    // xgettext:no-c-format
    statusWindow->addAttribute(Attributes::CRIT, _("% Critical"));
    statusWindow->addAttribute(Attributes::ATTACK_DELAY, _("Attack Delay"));
    statusWindow->addAttribute(Attributes::WALK_SPEED, _("Walk Delay"));
    statusWindow->addAttribute(Attributes::ATTACK_RANGE, _("Attack Range"));
    statusWindow->addAttribute(Attributes::ATTACK_SPEED, _("Damage per sec."));
    statusWindow->addAttribute(Attributes::KARMA, _("Karma"));
    statusWindow->addAttribute(Attributes::MANNER, _("Manner"));
}

void GeneralHandler::gameEnded() const
{
    if (socialWindow)
    {
        socialWindow->removeTab(Ea::taGuild);
        socialWindow->removeTab(Ea::taParty);
    }

    delete2(guildTab);
    delete2(partyTab);
}

}  // namespace EAthena
