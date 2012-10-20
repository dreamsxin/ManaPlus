/*
 *  The ManaPlus Client
 *  Copyright (C) 2012  The ManaPlus Developers
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

#ifndef GUI_WIDGET2_H
#define GUI_WIDGET2_H

#include "gui/theme.h"

class Widget2
{
    public:
        Widget2() :
            mPaletteOffset(0)
        {
        }

        Widget2(const Widget2 *const widget) :
            mPaletteOffset(widget ? widget->mPaletteOffset : 0)
        {
            checkPalette();
        }

        virtual ~Widget2()
        {
        }

        inline const gcn::Color &getThemeColor(const int type,
                                               const int alpha = 255) const
        {
            return Theme::getThemeColor(mPaletteOffset + type, alpha);
        }

        inline const gcn::Color &getThemeCharColor(const signed char c,
                                                   bool &valid) const
        {
            const int colorId = Theme::getThemeIdByChar(c, valid);
            if (valid)
                return Theme::getThemeColor(mPaletteOffset + colorId);
            else
                return Palette::BLACK;
        }

        virtual void setWidget2(const Widget2 *const widget)
        {
            mPaletteOffset = widget ? widget->mPaletteOffset : 0;
        }

        void setPalette(int palette)
        {
            mPaletteOffset = palette * Theme::THEME_COLORS_END;
            checkPalette();
            setWidget2(this);
        }

        void checkPalette()
        {
            if (mPaletteOffset < 0 || mPaletteOffset
                >= THEME_PALETTES * Theme::THEME_COLORS_END)
            {
                mPaletteOffset = 0;
            }
        }

    protected:
        int mPaletteOffset;
};

#endif
