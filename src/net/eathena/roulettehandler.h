/*
 *  The ManaPlus Client
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

#ifndef NET_EATHENA_ROULETTEHANDLER_H
#define NET_EATHENA_ROULETTEHANDLER_H

#include "net/roulettehandler.h"

#include "net/eathena/messagehandler.h"

namespace EAthena
{
class RouletteHandler final : public MessageHandler,
                              public Net::RouletteHandler
{
    public:
        RouletteHandler();

        A_DELETE_COPY(RouletteHandler)

        void handleMessage(Net::MessageIn &msg) override final;

    protected:
        static void processRouletteInfoAckType(Net::MessageIn &msg);

        static void processRouletteItemAck(Net::MessageIn &msg);
};

}  // namespace EAthena

#endif  // NET_EATHENA_ROULETTEHANDLER_H
