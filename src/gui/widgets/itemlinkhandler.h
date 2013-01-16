/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2013  The ManaPlus Developers
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

#ifndef ITEM_LINK_HANDLER_H
#define ITEM_LINK_HANDLER_H

#include "gui/widgets/linkhandler.h"

#include "localconsts.h"

class ItemPopup;

class ItemLinkHandler final : public LinkHandler
{
    public:
        ItemLinkHandler();

        A_DELETE_COPY(ItemLinkHandler)

        ~ItemLinkHandler();

        void handleLink(const std::string &link,
                        gcn::MouseEvent *event) override;

    private:
        ItemPopup *mItemPopup;
};

#endif
