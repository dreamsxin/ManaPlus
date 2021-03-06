/*
 *  The ManaPlus Client
 *  Copyright (C) 2011-2016  The ManaPlus Developers
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

#ifdef _MSC_VER
# include "msvc/config.h"
#elif defined(HAVE_CONFIG_H)
#include "config.h"
#endif  // _MSC_VER

#include "utils/paths.h"

#include "utils/physfstools.h"
#include "utils/stringutils.h"

#ifdef USE_X11
#include "utils/files.h"

#include "resources/resourcemanager/resourcemanager.h"
#endif  // USE_X11

#ifdef __native_client__
#include <limits.h>
#define realpath(N, R) strcpy(R, N)
#endif  // __native_client__

#ifdef WIN32
#include "utils/specialfolder.h"
#define realpath(N, R) _fullpath((R), (N), _MAX_PATH)
#elif defined __OpenBSD__
#include <limits>
#elif defined __native_client__
#include <limits.h>
#endif  // WIN32

#ifdef ANDROID
#ifdef USE_SDL2
#include <SDL_system.h>
#endif  // USE_SDL2
#endif  // ANDROID

#include "debug.h"

namespace
{
    std::string mPackageDir;
}  // namespace

std::string getRealPath(const std::string &str)
{
#if defined(__OpenBSD__) || defined(__ANDROID__) || defined(__native_client__)
    char *realPath = reinterpret_cast<char*>(calloc(PATH_MAX, sizeof(char)));
    if (!realPath)
        return "";
    realpath(str.c_str(), realPath);
#else  // defined(__OpenBSD__) || defined(__ANDROID__) ||
       // defined(__native_client__)

    char *realPath = realpath(str.c_str(), nullptr);
#endif  // defined(__OpenBSD__) || defined(__ANDROID__) ||
        // defined(__native_client__)

    if (!realPath)
        return "";
    std::string path = realPath;
    free(realPath);
    return path;
}

bool isRealPath(const std::string &str)
{
    return str == getRealPath(str);
}

bool checkPath(const std::string &path)
{
    if (path.empty())
        return true;
    return path.find("../") == std::string::npos
        && path.find("..\\") == std::string::npos
        && path.find("/..") == std::string::npos
        && path.find("\\..") == std::string::npos;
}

std::string &fixDirSeparators(std::string &str)
{
    if (dirSeparator[0] == '/')
        return str;

    return replaceAll(str, "/", "\\");
}

std::string removeLast(const std::string &str)
{
    size_t pos2 = str.rfind("/");
    const size_t pos3 = str.rfind("\\");
    if (pos3 != std::string::npos)
    {
        if (pos2 == std::string::npos || pos3 > pos2)
            pos2 = pos3;
    }
    if (pos2 != std::string::npos)
        return str.substr(0, pos2);
    else
        return str;
}

#ifdef WIN32
std::string getSelfName()
{
    return "manaplus.exe";
}

#elif defined(__APPLE__)
std::string getSelfName()
{
    return "manaplus.exe";
}

#elif defined __linux__ || defined __linux
#include <unistd.h>

std::string getSelfName()
{
    char buf[257];
    const ssize_t sz = readlink("/proc/self/exe", buf, 256);
    if (sz > 0 && sz < 256)
    {
        buf[sz] = 0;
        return buf;
    }
    else
    {
        return "";
    }
}

#else  // WIN32

std::string getSelfName()
{
    return "";
}

#endif  // WIN32

std::string getPicturesDir()
{
#ifdef WIN32
    std::string dir = getSpecialFolderLocation(CSIDL_MYPICTURES);
    if (dir.empty())
        dir = getSpecialFolderLocation(CSIDL_DESKTOP);
    return dir;
#elif defined USE_X11
    char *xdg = getenv("XDG_CONFIG_HOME");
    std::string file;
    if (!xdg)
    {
        file = std::string(PhysFs::getUserDir()).append(
            "/.config/user-dirs.dirs");
    }
    else
    {
        file = std::string(xdg).append("/user-dirs.dirs");
    }

    StringVect arr;
    Files::loadTextFileLocal(file, arr);
    FOR_EACH (StringVectCIter, it, arr)
    {
        std::string str = *it;
        if (findCutFirst(str, "XDG_PICTURES_DIR=\""))
        {
            str = str.substr(0, str.size() - 1);
            // use hack to replace $HOME var.
            // if in string other vars, fallback to default path
            replaceAll(str, "$HOME/", PhysFs::getUserDir());
            str = getRealPath(str);
            if (str.empty())
                str = std::string(PhysFs::getUserDir()).append("Desktop");
            return str;
        }
    }

    return std::string(PhysFs::getUserDir()).append("Desktop");
#else  // WIN32

    return std::string(PhysFs::getUserDir()).append("Desktop");
#endif  // WIN32
}

#ifdef ANDROID
std::string getSdStoragePath()
{
    return getenv("DATADIR2");
}
#endif  // ANDROID

std::string getPackageDir()
{
    return mPackageDir;
}

void setPackageDir(const std::string &dir)
{
    mPackageDir = dir;
}
