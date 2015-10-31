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

#ifndef RESOURCES_NPCDIALOGINFO_H
#define RESOURCES_NPCDIALOGINFO_H

#include "resources/npcbuttoninfo.h"
#include "resources/npcdialogmenuinfo.h"
#include "resources/npcimageinfo.h"
#include "resources/npcinventoryinfo.h"
#include "resources/npctextinfo.h"

#include "utils/stringvector.h"

#include "localconsts.h"

struct NpcDialogInfo final
{
    NpcDialogInfo() :
        menu(),
        inventory(),
        name(),
        hideText(false)
    {
    }

    NpcDialogMenuInfo menu;
    NpcInventoryInfo inventory;
    std::string name;
    bool hideText;
};

#endif  // RESOURCES_NPCDIALOGINFO_H