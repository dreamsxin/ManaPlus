/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
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

#ifndef SOUND_EFFECT_H
#define SOUND_EFFECT_H

#include "resources/resource.h"

#include <SDL_mixer.h>

#include "localconsts.h"

/**
 * Defines a class for loading and storing sound effects.
 */
class SoundEffect final : public Resource
{
    public:
        A_DELETE_COPY(SoundEffect)

        /**
         * Destructor.
         */
        virtual ~SoundEffect();

        /**
         * Loads a sample from a buffer in memory.
         *
         * @param rw         The SDL_RWops to load the sample data from.
         *
         * @return <code>NULL</code> if the an error occurred, a valid pointer
         *         otherwise.
         */
        static Resource *load(SDL_RWops *const rw);

        /**
         * Plays the sample.
         *
         * @param loops     Number of times to repeat the playback.
         * @param volume    Sample playback volume.
         * @param channel   Sample playback channel.
         *
         * @return <code>true</code> if the playback started properly
         *         <code>false</code> otherwise.
         */
        virtual bool play(const int loops, const int volume,
                          const int channel = -1);

    protected:
        /**
         * Constructor.
         */
        SoundEffect(Mix_Chunk *const soundEffect) :
            Resource(),
            mChunk(soundEffect)
        { }

        Mix_Chunk *mChunk;
};

#endif // SOUND_EFFECT_H
