/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2016  The ManaPlus Developers
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

#include "resources/sprite/animatedsprite.h"

#include "catch.hpp"
#include "client.h"
#include "graphicsmanager.h"

#include "const/resources/spriteaction.h"

#include "being/actorsprite.h"

#include "gui/theme.h"

#include "resources/sdlimagehelper.h"

#include "resources/animation/animation.h"

#include "resources/resourcemanager/resourcemanager.h"

#include "utils/env.h"
#include "utils/mrand.h"
#include "utils/physfstools.h"

#ifndef USE_SDL2
#include <SDL.h>
#endif  // USE_SDL2

#include "debug.h"

TEST_CASE("AnimatedSprite tests", "animatedsprite")
{
    setEnv("SDL_VIDEODRIVER", "dummy");

    initRand();
    client = new Client;
    PHYSFS_init("manaplus");
    dirSeparator = "/";
    XML::initXML();
    SDL_Init(SDL_INIT_VIDEO);
    logger = new Logger();
    ResourceManager::init();
    resourceManager->addToSearchPath("data", Append_false);
    resourceManager->addToSearchPath("../data", Append_false);
    theme = new Theme;
    Theme::selectSkin();
    imageHelper = new SDLImageHelper();
#ifdef USE_SDL2
    SDLImageHelper::setRenderer(graphicsManager.createRenderer(
        graphicsManager.createWindow(640, 480, 0,
        SDL_WINDOW_SHOWN | SDL_SWSURFACE), SDL_RENDERER_SOFTWARE));
#else  // USE_SDL2

    graphicsManager.createWindow(640, 480, 0, SDL_ANYFORMAT | SDL_SWSURFACE);
#endif  // USE_SDL2

    ActorSprite::load();

    SECTION("basic test 1")
    {
        AnimatedSprite *sprite = AnimatedSprite::load(
            "graphics/sprites/error.xml", 0);
        sprite->play(SpriteAction::DEFAULT);

        REQUIRE_FALSE(sprite == nullptr);
        REQUIRE_FALSE(sprite->getSprite() == nullptr);
        REQUIRE_FALSE(sprite->getAnimation() == nullptr);
        REQUIRE_FALSE(sprite->getFrame() == nullptr);
        REQUIRE(0 == sprite->getFrameIndex());
        REQUIRE(0 == sprite->getFrameTime());
        REQUIRE(false == sprite->update(1));
        REQUIRE(0 == sprite->getFrameTime());
        REQUIRE(false == sprite->update(11));
        REQUIRE(10 == sprite->getFrameTime());
        REQUIRE(0 == sprite->getFrameIndex());
        delete sprite;
    }

    SECTION("basic test 2")
    {
        AnimatedSprite *sprite = AnimatedSprite::load(
            "graphics/sprites/test.xml", 0);
        sprite->play(SpriteAction::STAND);

        REQUIRE(10 == const_cast<Animation*>(sprite->getAnimation())
            ->getFrames().size());

        REQUIRE_FALSE(nullptr == sprite);

        REQUIRE(false == sprite->update(1));
        REQUIRE(0 == sprite->getFrameTime());
        REQUIRE(10 == sprite->getFrame()->delay);

        REQUIRE(false == sprite->update(1 + 10));
        REQUIRE(0 == sprite->getFrameIndex());
        REQUIRE(10 == sprite->getFrameTime());

        REQUIRE(true == sprite->update(1 + 10 + 5));
        REQUIRE(1 == sprite->getFrameIndex());
        REQUIRE(5 == sprite->getFrameTime());

        REQUIRE(false == sprite->update(1 + 10 + 5));
        REQUIRE(1 == sprite->getFrameIndex());
        REQUIRE(5 == sprite->getFrameTime());

        REQUIRE(false == sprite->update(1 + 10 + 20));
        REQUIRE(1 == sprite->getFrameIndex());
        REQUIRE(20 == sprite->getFrameTime());

        REQUIRE(true == sprite->update(1 + 10 + 20 + 1));
        REQUIRE(2 == sprite->getFrameIndex());
        REQUIRE(1 == sprite->getFrameTime());

        REQUIRE(false == sprite->update(1 + 10 + 20 + 10));
        REQUIRE(2 == sprite->getFrameIndex());
        REQUIRE(10 == sprite->getFrameTime());

        REQUIRE(true == sprite->update(1 + 10 + 20 + 10 + 1));
        REQUIRE(4 == sprite->getFrameIndex());
        REQUIRE(1 == sprite->getFrameTime());

        REQUIRE(false == sprite->update(1 + 10 + 20 + 10 + 25));
        REQUIRE(4 == sprite->getFrameIndex());
        REQUIRE(25 == sprite->getFrameTime());

        REQUIRE(true == sprite->update(1 + 10 + 20 + 10 + 25 + 1));
        REQUIRE(6 == sprite->getFrameIndex());
        REQUIRE(1 == sprite->getFrameTime());

        REQUIRE(true == sprite->update(1 + 10 + 20 + 10 + 25 + 10 + 1));
        REQUIRE(8 == sprite->getFrameIndex());
        REQUIRE(1 == sprite->getFrameTime());

        REQUIRE(true == sprite->update(1 + 10 + 20 + 10 + 25 + 10 + 10 + 1));
        REQUIRE(4 == sprite->getFrameIndex());
        REQUIRE(1 == sprite->getFrameTime());
        delete sprite;
    }

    SECTION("basic test 3")
    {
        AnimatedSprite *sprite2 = AnimatedSprite::load(
            "graphics/sprites/test.xml", 0);
        sprite2->play(SpriteAction::SIT);

        REQUIRE(false == sprite2->update(1));
        REQUIRE(2 == const_cast<Animation*>(sprite2->getAnimation())
            ->getFrames().size());
        REQUIRE(0 == sprite2->getFrameTime());
        REQUIRE(85 == sprite2->getFrame()->delay);

        REQUIRE(true == sprite2->update(1 + 10 + 20 + 10 + 25 + 10 + 10 + 1));
        REQUIRE(1 == sprite2->getFrameIndex());
        REQUIRE(1 == sprite2->getFrameTime());
        delete sprite2;
    }

    delete client;
    client = nullptr;
}
