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

#ifndef GUI_WINDOWS_MAILEDIT_H
#define GUI_WINDOWS_MAILEDIT_H

#ifdef EATHENA_SUPPORT

#include "gui/widgets/window.h"

#include "listeners/actionlistener.h"

class Button;
class Label;
class TextField;

class MailEdit final : public Window,
                       public ActionListener
{
    public:
        MailEdit();

        A_DELETE_COPY(MailEdit)

        ~MailEdit();

        void action(const ActionEvent &event) override final;

    private:
        Button *mSendButton;
        Button *mCloseButton;
        Label *mToLabel;
        Label *mSubjectLabel;
        Label *mMessageLabel;
        TextField *mToField;
        TextField *mSubjectField;
        TextField *mMessageField;
};

#endif  // EATHENA_SUPPORT
#endif  // GUI_WINDOWS_MAILEDIT_H