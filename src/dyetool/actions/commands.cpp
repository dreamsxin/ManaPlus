/*
 *  The ManaPlus Client
 *  Copyright (C) 2012-2015  The ManaPlus Developers
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

#include "actions/commands.h"

#include "actions/actiondef.h"

#include "debug.h"

namespace Actions
{

impHandlerVoid(chatAnnounce)
impHandlerVoid(chatIgnore)
impHandlerVoid(chatUnignore)
impHandlerVoid(chatErase)
impHandlerVoid(chatFriend)
impHandlerVoid(chatDisregard)
impHandlerVoid(chatNeutral)
impHandlerVoid(chatBlackList)
impHandlerVoid(chatEnemy)
impHandlerVoid(chatNuke)
impHandlerVoid(chatAdd)
impHandlerVoid(present)
impHandlerVoid(printAll)
impHandlerVoid(move)
impHandlerVoid(setTarget)
impHandlerVoid(commandOutfit)
impHandlerVoid(commandEmote)
impHandlerVoid(awayMessage)
impHandlerVoid(pseudoAway)
impHandlerVoid(follow)
impHandlerVoid(navigate)
impHandlerVoid(navigateTo)
impHandlerVoid(moveCamera)
impHandlerVoid(imitation)
impHandlerVoid(sendMail)
impHandlerVoid(info)
impHandlerVoid(wait)
impHandlerVoid(addPriorityAttack)
impHandlerVoid(addAttack)
impHandlerVoid(removeAttack)
impHandlerVoid(addIgnoreAttack)
impHandlerVoid(setDrop)
impHandlerVoid(url)
impHandlerVoid(openUrl)
impHandlerVoid(execute)
impHandlerVoid(enableHighlight)
impHandlerVoid(disableHighlight)
impHandlerVoid(dontRemoveName)
impHandlerVoid(removeName)
impHandlerVoid(disableAway)
impHandlerVoid(enableAway)
impHandlerVoid(testParticle)
impHandlerVoid(talkRaw)
impHandlerVoid(gm)
impHandlerVoid(hack)
impHandlerVoid(debugSpawn)
impHandlerVoid(serverIgnoreWhisper)
impHandlerVoid(serverUnIgnoreWhisper)
impHandlerVoid(setHomunculusName)
impHandlerVoid(fireHomunculus)
impHandlerVoid(leaveParty)
impHandlerVoid(warp)
impHandlerVoid(homunTalk)
impHandlerVoid(homunEmote)
impHandlerVoid(commandHomunEmote)
impHandlerVoid(createPublicChatRoom)
impHandlerVoid(joinChatRoom)
impHandlerVoid(leaveChatRoom)
impHandlerVoid(confSet)
impHandlerVoid(serverConfSet)
impHandlerVoid(confGet)
impHandlerVoid(serverConfGet)

}  // namespace Actions