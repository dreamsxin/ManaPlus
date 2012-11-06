/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
 *  Copyright (C) 2011-2012  The ManaPlus developers
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

#include "gui/whoisonline.h"

#include <SDL.h>
#include <SDL_thread.h>
#include <vector>
#include <algorithm>

#include "gui/socialwindow.h"
#include "gui/viewport.h"

#include "gui/widgets/button.h"
#include "gui/widgets/browserbox.h"
#include "gui/widgets/scrollarea.h"
#include "gui/widgets/chattab.h"

#include "actorspritemanager.h"
#include "client.h"
#include "configuration.h"
#include "localplayer.h"
#include "playerrelations.h"
#include "main.h"

#include "net/download.h"
#include "net/net.h"
#include "net/playerhandler.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

// Curl should be included after Guichan to avoid Windows redefinitions
#include <curl/curl.h>

#include "debug.h"

#ifdef free
#undef free
#endif

#ifdef malloc
#undef malloc
#endif


class NameFunctuator final
{
    public:
        bool operator()(const OnlinePlayer *left,
                        const OnlinePlayer *right) const
        {
            return (compareStrI(left->getNick(), right->getNick()) < 0);
        }
} nameCompare;

WhoIsOnline::WhoIsOnline():
    Window(_("Who Is Online - Updating"), false, nullptr, "whoisonline.xml"),
    mThread(nullptr),
    mDownloadStatus(UPDATE_LIST),
    mDownloadComplete(true),
    mDownloadedBytes(0),
    mMemoryBuffer(nullptr),
    mCurlError(new char[CURL_ERROR_SIZE]),
    mAllowUpdate(true),
    mShowLevel(false),
    mGroupFriends(true)
{
    mCurlError[0] = 0;
    setWindowName("WhoIsOnline");

    const int h = 350;
    const int w = 200;
    setDefaultSize(w, h, ImageRect::CENTER);
//    setContentSize(w, h);
    setVisible(false);
    setCloseButton(true);
    setResizable(true);
    setStickyButtonLock(true);
    setSaveVisible(true);

    mUpdateButton = new Button(this, _("Update"), "update", this);
    mUpdateButton->setEnabled(false);
    mUpdateButton->setDimension(gcn::Rectangle(5, 5, w - 10, 20 + 5));

    mBrowserBox = new BrowserBox(this);
    mScrollArea = new ScrollArea(mBrowserBox, false);
    mBrowserBox->setOpaque(false);
    mBrowserBox->setHighlightMode(BrowserBox::BACKGROUND);
    mScrollArea->setDimension(gcn::Rectangle(5, 20 + 10, w - 10, h - 10 - 30));
    mScrollArea->setSize(w - 10, h - 10 - 30);
    mBrowserBox->setLinkHandler(this);

    add(mUpdateButton);
    add(mScrollArea);

    mUpdateTimer = 0;
    setLocationRelativeTo(getParent());

    loadWindowState();
    enableVisibleSound(true);

    download();

    config.addListener("updateOnlineList", this);
    config.addListener("groupFriends", this);
    mUpdateOnlineList = config.getBoolValue("updateOnlineList");
    mGroupFriends = config.getBoolValue("groupFriends");
}

WhoIsOnline::~WhoIsOnline()
{
    config.removeListeners(this);

    if (mThread && SDL_GetThreadID(mThread))
        SDL_WaitThread(mThread, nullptr);

    free(mMemoryBuffer);
    mMemoryBuffer = nullptr;

    // Remove possibly leftover temporary download
    delete []mCurlError;

    for (std::set<OnlinePlayer*>::iterator itd = mOnlinePlayers.begin(),
         itd_end = mOnlinePlayers.end(); itd != itd_end; ++ itd)
    {
        delete *itd;
    }
    mOnlinePlayers.clear();
    mOnlineNicks.clear();
}

void WhoIsOnline::handleLink(const std::string& link, gcn::MouseEvent *event)
{
    if (!event || event->getButton() == gcn::MouseEvent::LEFT)
    {
        if (chatWindow)
        {
            if (config.getBoolValue("whispertab"))
                chatWindow->localChatInput("/q " + link);
            else
                chatWindow->addInputText("/w \"" + link + "\" ");
        }
    }
    else if (event->getButton() == gcn::MouseEvent::RIGHT)
    {
        if (player_node && link == player_node->getName())
            return;

        if (viewport)
        {
            if (actorSpriteManager)
            {
                Being* being = actorSpriteManager->findBeingByName(
                    link, Being::PLAYER);

                if (being && viewport)
                {
                    viewport->showPopup(being);
                    return;
                }
            }
            viewport->showPlayerPopup(link);
        }
    }
}

void WhoIsOnline::updateWindow(std::vector<OnlinePlayer*> &friends,
                               std::vector<OnlinePlayer*> &neutral,
                               std::vector<OnlinePlayer*> &disregard,
                               std::vector<OnlinePlayer*> enemy,
                               size_t numOnline)
{
    //Set window caption
    setCaption(_("Who Is Online - ") + toString(numOnline));

    //List the online people
    std::sort(friends.begin(), friends.end(), nameCompare);
    std::sort(neutral.begin(), neutral.end(), nameCompare);
    std::sort(disregard.begin(), disregard.end(), nameCompare);
    bool addedFromSection(false);
    for (size_t i = 0, sz = friends.size(); i < sz; i++)
    {
        mBrowserBox->addRow(friends.at(i)->getText());
        addedFromSection = true;
    }
    if (addedFromSection == true)
    {
        mBrowserBox->addRow("---");
        addedFromSection = false;
    }
    for (size_t i = 0, sz = enemy.size(); i < sz; i++)
    {
        mBrowserBox->addRow(enemy.at(i)->getText());
        addedFromSection = true;
    }
    if (addedFromSection == true)
    {
        mBrowserBox->addRow("---");
        addedFromSection = false;
    }
    for (size_t i = 0, sz = neutral.size(); i < sz; i++)
    {
        mBrowserBox->addRow(neutral.at(i)->getText());
        addedFromSection = true;
    }
    if (addedFromSection == true && !disregard.empty())
    {
        mBrowserBox->addRow("---");
//        addedFromSection = false;
    }
    for (size_t i = 0, sz = disregard.size(); i < sz; i++)
        mBrowserBox->addRow(disregard.at(i)->getText());

    if (mScrollArea->getVerticalMaxScroll() <
        mScrollArea->getVerticalScrollAmount())
    {
        mScrollArea->setVerticalScrollAmount(
            mScrollArea->getVerticalMaxScroll());
    }
}

void WhoIsOnline::loadList(std::vector<OnlinePlayer*> &list)
{
    mBrowserBox->clearRows();
    const size_t numOnline = list.size();
    std::vector<OnlinePlayer*> friends;
    std::vector<OnlinePlayer*> neutral;
    std::vector<OnlinePlayer*> disregard;
    std::vector<OnlinePlayer*> enemy;

    for (std::set<OnlinePlayer*>::iterator itd = mOnlinePlayers.begin(),
         itd_end = mOnlinePlayers.end(); itd != itd_end; ++ itd)
    {
        delete *itd;
    }
    mOnlinePlayers.clear();
    mOnlineNicks.clear();

    mShowLevel = config.getBoolValue("showlevel");

    for (std::vector<OnlinePlayer*>::const_iterator it = list.begin(),
         it_end = list.end(); it != it_end; ++ it)
    {
        OnlinePlayer *player = *it;
        std::string nick = player->getNick();
        mOnlinePlayers.insert(player);
        mOnlineNicks.insert(nick);

        if (!mShowLevel)
            player->setLevel(0);

        switch (player_relations.getRelation(nick))
        {
            case PlayerRelation::NEUTRAL:
            default:
                player->setText("0");
                neutral.push_back(player);
                break;

            case PlayerRelation::FRIEND:
                player->setText("2");
                if (mGroupFriends)
                    friends.push_back(player);
                else
                    neutral.push_back(player);
                break;

            case PlayerRelation::DISREGARDED:
            case PlayerRelation::BLACKLISTED:
                player->setText("8");
                disregard.push_back(player);
                break;

            case PlayerRelation::ENEMY2:
                player->setText("1");
                enemy.push_back(player);
                break;

            case PlayerRelation::IGNORED:
            case PlayerRelation::ERASED:
                //Ignore the ignored.
                break;
        }
    }

    updateWindow(friends, neutral, disregard, enemy, numOnline);
    if (!mOnlineNicks.empty())
    {
        if (chatWindow)
            chatWindow->updateOnline(mOnlineNicks);
        if (socialWindow)
            socialWindow->updateActiveList();
    }
}

void WhoIsOnline::loadWebList()
{
    if (!mMemoryBuffer)
        return;

    // Reallocate and include terminating 0 character
    mMemoryBuffer = static_cast<char*>(
        realloc(mMemoryBuffer, mDownloadedBytes + 1));
    mMemoryBuffer[mDownloadedBytes] = '\0';

    mBrowserBox->clearRows();
    bool listStarted(false);
    std::string lineStr;
    int numOnline(0);
    std::vector<OnlinePlayer*> friends;
    std::vector<OnlinePlayer*> neutral;
    std::vector<OnlinePlayer*> disregard;
    std::vector<OnlinePlayer*> enemy;

    // Tokenize and add each line separately
    char *line = strtok(mMemoryBuffer, "\n");
    const std::string gmText = "(GM)";

    for (std::set<OnlinePlayer*>::iterator itd = mOnlinePlayers.begin(),
         itd_end = mOnlinePlayers.end(); itd != itd_end; ++ itd)
    {
        delete *itd;
    }

    mOnlinePlayers.clear();
    mOnlineNicks.clear();

    mShowLevel = config.getBoolValue("showlevel");

    while (line)
    {
        std::string nick;
        lineStr = line;
        trim(lineStr);
        if (listStarted == true)
        {
            if (lineStr.find(" users are online.") == std::string::npos)
            {
                int level = 0;

                if (lineStr.length() > 24)
                {
                    nick = lineStr.substr(0, 24);
                    lineStr = lineStr.substr(25);
                }
                else
                {
                    nick = lineStr;
                    lineStr = "";
                }
                trim(nick);

                bool isGM(false);
                size_t pos = lineStr.find(gmText, 0);
                if (pos != std::string::npos)
                {
                    lineStr = lineStr.substr(pos + gmText.length());
                    isGM = true;
                }

                trim(lineStr);
                pos = lineStr.find("/", 0);

                if (pos != std::string::npos)
                    lineStr = lineStr.substr(0, pos);

                if (!lineStr.empty())
                    level = atoi(lineStr.c_str());

                if (actorSpriteManager)
                {
                    Being *being = actorSpriteManager->findBeingByName(
                        nick, Being::PLAYER);
                    if (being)
                    {
                        if (level > 0)
                        {
                            being->setLevel(level);
                            being->updateName();
                        }
                        else
                        {
                            if (being->getLevel() > 1)
                                level = being->getLevel();
                        }
                    }
                }


                if (!mShowLevel)
                    level = 0;

                OnlinePlayer *player = new OnlinePlayer(nick,
                    static_cast<signed char>(255), level,
                    GENDER_UNSPECIFIED, -1);
                mOnlinePlayers.insert(player);
                mOnlineNicks.insert(nick);

                if (isGM)
                    player->setIsGM(true);

                numOnline++;
                switch (player_relations.getRelation(nick))
                {
                    case PlayerRelation::NEUTRAL:
                    default:
                        player->setText("0");
                        neutral.push_back(player);
                        break;

                    case PlayerRelation::FRIEND:
                        player->setText("2");
                        if (mGroupFriends)
                            friends.push_back(player);
                        else
                            neutral.push_back(player);
                        break;

                    case PlayerRelation::DISREGARDED:
                    case PlayerRelation::BLACKLISTED:
                        player->setText("8");
                        disregard.push_back(player);
                        break;

                    case PlayerRelation::ENEMY2:
                        player->setText("1");
                        enemy.push_back(player);
                        break;

                    case PlayerRelation::IGNORED:
                    case PlayerRelation::ERASED:
                        //Ignore the ignored.
                        break;
                }
            }
        }
        else if (lineStr.find("------------------------------")
                 != std::string::npos)
        {
            listStarted = true;
        }
        line = strtok(nullptr, "\n");
    }

    updateWindow(friends, neutral, disregard, enemy, numOnline);

    // Free the memory buffer now that we don't need it anymore
    free(mMemoryBuffer);
    mMemoryBuffer = nullptr;
}

size_t WhoIsOnline::memoryWrite(void *ptr, size_t size,
                                size_t nmemb, FILE *stream)
{
    if (!stream)
        return 0;

    WhoIsOnline *wio = reinterpret_cast<WhoIsOnline *>(stream);
    const size_t totalMem = size * nmemb;
    wio->mMemoryBuffer = static_cast<char*>(realloc(wio->mMemoryBuffer,
        wio->mDownloadedBytes + totalMem));
    if (wio->mMemoryBuffer)
    {
        memcpy(&(wio->mMemoryBuffer[wio->mDownloadedBytes]), ptr, totalMem);
        wio->mDownloadedBytes += static_cast<int>(totalMem);
    }

    return totalMem;
}

int WhoIsOnline::downloadThread(void *ptr)
{
    int attempts = 0;
    WhoIsOnline *wio = reinterpret_cast<WhoIsOnline *>(ptr);
    CURL *curl;
    CURLcode res;

    std::string url(Client::getServerName() + "/online.txt");

    while (attempts < 1 && !wio->mDownloadComplete)
    {
        curl = curl_easy_init();

        if (curl)
        {
            if (!wio->mAllowUpdate)
            {
                curl_easy_cleanup(curl);
                curl = nullptr;
                break;
            }
            wio->mDownloadedBytes = 0;
            curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                                   WhoIsOnline::memoryWrite);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, ptr);

            curl_easy_setopt(curl, CURLOPT_USERAGENT,
                strprintf(PACKAGE_EXTENDED_VERSION,
                branding.getStringValue("appName").c_str()).c_str());

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, wio->mCurlError);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);
            curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, ptr);
            curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
            curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 7);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
            Net::Download::addProxy(curl);

            struct curl_slist *pHeaders = nullptr;
            // Make sure the resources2.txt and news.txt aren't cached,
            // in order to always get the latest version.
            pHeaders = curl_slist_append(pHeaders, "pragma: no-cache");
            pHeaders =
                curl_slist_append(pHeaders, "Cache-Control: no-cache");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pHeaders);

            if ((res = curl_easy_perform(curl)) != 0)
            {
                wio->mDownloadStatus = UPDATE_ERROR;
                switch (res)
                {
                    case CURLE_COULDNT_CONNECT:
                    default:
                        std::cerr << "curl error "
                                  << static_cast<unsigned>(res) << ": "
                                  << wio->mCurlError << " host: "
                                  << url.c_str() << std::endl;
                    break;
                }
                attempts++;
                curl_easy_cleanup(curl);
                curl_slist_free_all(pHeaders);
                curl = nullptr;
                continue;
            }

            curl_easy_cleanup(curl);
            curl_slist_free_all(pHeaders);

            // It's stored in memory, we're done
            wio->mDownloadComplete = true;
        }
        if (!wio->mAllowUpdate)
            break;
        attempts++;
    }

    if (!wio->mDownloadComplete)
        wio->mDownloadStatus = UPDATE_ERROR;

//    wio->mThread = 0;
    return 0;
}

void WhoIsOnline::download()
{
    if (serverVersion < 3)
    {
        mDownloadComplete = true;
        if (mThread && SDL_GetThreadID(mThread))
            SDL_WaitThread(mThread, nullptr);

        mDownloadComplete = false;
        mThread = SDL_CreateThread(WhoIsOnline::downloadThread, this);

        if (mThread == nullptr)
            mDownloadStatus = UPDATE_ERROR;
    }
    else
    {
        if (Client::limitPackets(PACKET_ONLINELIST))
            Net::getPlayerHandler()->requestOnlineList();
    }
}

void WhoIsOnline::logic()
{
    // Update Scroll logic
    mScrollArea->logic();
}

void WhoIsOnline::slowLogic()
{
    if (!mAllowUpdate)
        return;

    BLOCK_START("WhoIsOnline::slowLogic")
    if (mUpdateTimer == 0)
        mUpdateTimer = cur_time;

    double timeDiff = difftime(cur_time, mUpdateTimer);
    int timeLimit = isVisible() ? 20 : 120;

    if (mUpdateOnlineList && timeDiff >= timeLimit
        && mDownloadStatus != UPDATE_LIST)
    {
        if (mDownloadComplete == true)
        {
            setCaption(_("Who Is Online - Updating"));
            mUpdateTimer = 0;
            mDownloadStatus = UPDATE_LIST;
            download();
        }
    }

    switch (mDownloadStatus)
    {
        case UPDATE_ERROR:
            mBrowserBox->clearRows();
            mBrowserBox->addRow("##1Failed to fetch the online list!");
            mBrowserBox->addRow(mCurlError);
            mDownloadStatus = UPDATE_COMPLETE;
            setCaption(_("Who Is Online - error"));
            mUpdateButton->setEnabled(true);
            mUpdateTimer = cur_time + 240;
            mDownloadComplete = true;
            updateSize();
            break;
        case UPDATE_LIST:
            if (mDownloadComplete == true)
            {
                loadWebList();
                mDownloadStatus = UPDATE_COMPLETE;
                mUpdateButton->setEnabled(true);
                mUpdateTimer = 0;
                updateSize();
                if (!mOnlineNicks.empty())
                {
                    if (chatWindow)
                        chatWindow->updateOnline(mOnlineNicks);
                    if (socialWindow)
                        socialWindow->updateActiveList();
                }
            }
            break;
        case UPDATE_COMPLETE:
        default:
            break;
    }
    BLOCK_END("WhoIsOnline::slowLogic")
}

void WhoIsOnline::action(const gcn::ActionEvent &event)
{
    if (event.getId() == "update")
    {
        if (serverVersion < 3)
        {
            if (mDownloadStatus == UPDATE_COMPLETE)
            {
                mUpdateTimer = cur_time - 20;
                if (mUpdateButton)
                    mUpdateButton->setEnabled(false);
                setCaption(_("Who Is Online - Update"));
                if (mThread && SDL_GetThreadID(mThread))
                {
                    SDL_WaitThread(mThread, nullptr);
                    mThread = nullptr;
                }
                mDownloadComplete = true;
            }
        }
        else
        {
            if (Client::limitPackets(PACKET_ONLINELIST))
            {
                mUpdateTimer = cur_time;
                Net::getPlayerHandler()->requestOnlineList();
            }
        }
    }
}

void WhoIsOnline::widgetResized(const gcn::Event &event)
{
    Window::widgetResized(event);
    updateSize();
}

void WhoIsOnline::updateSize()
{
    if (mDownloadStatus == UPDATE_COMPLETE)
    {
        const gcn::Rectangle area = getChildrenArea();
        if (mUpdateButton)
            mUpdateButton->setWidth(area.width - 10);

        if (mScrollArea)
            mScrollArea->setSize(area.width - 10, area.height - 10 - 30);
    }
}

const std::string WhoIsOnline::prepareNick(std::string nick, int level,
                                           std::string color) const
{
    if (mShowLevel && level > 1)
    {
        return strprintf("@@%s|##%s%s (%d)@@", nick.c_str(),
                         color.c_str(), nick.c_str(), level);
    }
    else
    {
        return strprintf("@@%s|##%s%s@@", nick.c_str(),
                         color.c_str(), nick.c_str());
    }
}

void WhoIsOnline::optionChanged(const std::string &name)
{
    if (name == "updateOnlineList")
        mUpdateOnlineList = config.getBoolValue("updateOnlineList");
    else if (name == "groupFriends")
        mGroupFriends = config.getBoolValue("groupFriends");
}

void OnlinePlayer::setText(std::string color)
{
    mText = "";

    if (mStatus != 255 && actorSpriteManager)
    {
        Being *being = actorSpriteManager->findBeingByName(
            mNick, Being::PLAYER);
        if (being)
        {
            being->setState(mStatus);
            // for now highlight versions > 3
            if (mVersion > 3)
                being->setAdvanced(true);
        }
    }

    if ((mStatus != 255 && mStatus & Being::FLAG_GM) || mIsGM)
        mText += "(GM) ";

    if (mLevel > 0)
        mText += strprintf("%d", mLevel);

    if (mGender == GENDER_FEMALE)
        mText += "\u2640";
    else if (mGender == GENDER_MALE)
        mText += "\u2642";

    if (mStatus > 0 && mStatus != 255)
    {
        if (mStatus & Being::FLAG_SHOP)
            mText += "$";
        if (mStatus & Being::FLAG_AWAY)
        {
            // TRANSLATORS: this away status writed in player nick
            mText += _("A");
        }
        if (mStatus & Being::FLAG_INACTIVE)
        {
            // TRANSLATORS: this inactive status writed in player nick
            mText += _("I");
        }

        if (mStatus & Being::FLAG_GM && color == "0")
            color = "2";
    }
    else if (mIsGM && color == "0")
    {
        color = "2";
    }

    if (mVersion > 0)
        mText += strprintf(" - %d", mVersion);

    mText = strprintf("@@%s|##%s%s %s@@", mNick.c_str(),
        color.c_str(), mNick.c_str(), mText.c_str());
}
