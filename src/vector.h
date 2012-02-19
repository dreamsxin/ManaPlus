/*
 *  The ManaPlus Client
 *  Copyright (C) 2007-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2012  The ManaPlus Developers
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

#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

#include <iostream>

/**
 * Vector class. Represents either a 3D point in space, a velocity or a force.
 * Provides several convenient operator overloads.
 */
class Vector
{
    public:
        /**
         * Constructor.
         */
        Vector():
            x(0.0f),
            y(0.0f),
            z(0.0f)
        {}

        /**
         * Constructor.
         */
        Vector(float x0, float y0, float z0 = 0.0f):
            x(x0),
            y(y0),
            z(z0)
        {}

        /**
         * Copy constructor.
         */
        Vector(const Vector &v):
            x(v.x),
            y(v.y),
            z(v.z)
        {}

        /**
         * Returns true if all coordinates are set to 0, otherwise returns
         * false.
         */
        bool isNull() const
        {
            return x == 0.0f && y == 0.0f && z == 0.0f;
        }

        /**
         * Scale vector operator.
         */
        Vector operator*(float c) const
        {
            return Vector(x * c,
                          y * c,
                          z * c);
        }

        /**
         * In-place scale vector operator.
         */
        Vector &operator*=(float c)
        {
            x *= c;
            y *= c;
            z *= c;
            return *this;
        }

        /**
         * Scale vector operator.
         */
        Vector operator/(float c) const
        {
            return Vector(x / c,
                          y / c,
                          z / c);
        }

        /**
         * In-place scale vector operator.
         */
        Vector &operator/=(float c)
        {
            x /= c;
            y /= c;
            z /= c;
            return *this;
        }

        /**
         * Add vector operator.
         */
        Vector operator+(const Vector &v) const
        {
            return Vector(x + v.x,
                          y + v.y,
                          z + v.z);
        }

        /**
         * In-place add vector operator.
         */
        Vector &operator+=(const Vector &v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
        }

        /**
         * Subtract vector operator.
         */
        Vector operator-(const Vector &v) const
        {
            return Vector(x - v.x,
                          y - v.y,
                          z - v.z);
        }

        /**
         * In-place subtract vector operator.
         */
        Vector &operator-=(const Vector &v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
            return *this;
        }

        /**
         * Returns the length of this vector. This method does a relatively
         * slow square root.
         */
        float length() const
        {
            return sqrtf(x * x + y * y + z * z);
        }

        /**
         * Returns the squared length of this vector. Avoids the square root.
         */
        float squaredLength() const
        {
            return x * x + y * y + z * z;
        }

        /**
         * Returns the manhattan length of this vector.
         */
        float manhattanLength() const
        {
            return fabsf(x) + fabsf(y) + fabsf(z);
        }

        /**
         * Returns a normalized version of this vector. This is a unit vector
         * running parallel to it.
         */
        Vector normalized() const
        {
            const float len = length();
            return Vector(x / len, y / len, z / len);
        }

        float x, y, z;
};

/**
 * Appends a string representation of a vector to the output stream.
 */
std::ostream& operator <<(std::ostream &os, const Vector &v);

#endif // VECTOR_H
