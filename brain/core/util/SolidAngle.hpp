/*  Elysia Utilities -- Math Library
 *  SolidAngle.hpp
 *
 *  Copyright (c) 2009, Ewen Cheslack-Postava
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name of libprox nor the names of its contributors may
 *    be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ELYSIA_SOLID_ANGLE_HPP_
#define _ELYSIA_SOLID_ANGLE_HPP_

namespace Elysia {

class ELYSIA_EXPORT SolidAngle {
public:
    static const float Pi;
    static const SolidAngle Min;
    static const SolidAngle Max;

    // Sometimes you need the float value instead of the SolidAngle
    // directly. These are the values used to generate SolidAngle::Min and
    // SolidAngle::Max
    static const float MinVal;
    static const float MaxVal;

    SolidAngle();
    explicit SolidAngle(float sa);
    SolidAngle(const SolidAngle& cpy);
    ~SolidAngle();

    /// Get the solid angle represented by the circular area with the given vector to its center and radius
    static SolidAngle fromCenterRadius(const Vector3<float>& to_center, float radius);

    SolidAngle operator+(const SolidAngle& rhs) const;
    SolidAngle& operator+=(const SolidAngle& rhs);
    SolidAngle operator-(const SolidAngle& rhs) const;
    SolidAngle& operator-=(const SolidAngle& rhs);

    SolidAngle operator*(float rhs) const;
    SolidAngle& operator*=(float rhs);

    SolidAngle operator/(float rhs) const;
    SolidAngle& operator/=(float rhs);

    bool operator<(const SolidAngle& rhs) const;
    bool operator==(const SolidAngle& rhs) const;
    bool operator<=(const SolidAngle& rhs) const;
    bool operator>(const SolidAngle& rhs) const;
    bool operator>=(const SolidAngle& rhs) const;
    bool operator!=(const SolidAngle& rhs) const;

    float asFloat() const;

    // Compares this solid angle to one generated at an offset + radius, where
    // the offset is specified by distance^2. Allows more efficient comparison
    // than computing real solid angle values.
    //
    // Returns a positive value if this solid angle is <= the solid angle
    // computed from the given center offset and radius, and -1 if this solid
    // angle is greater than the one that would be computed from the given
    // center offset and radius. The odd interface allows us to both a) compare
    // the values and b) return a value for the given bounds + radius that can
    // be used for rating. Therefore, the positive return values must have the
    // same sorted order as if the actual solid angle of the given parameters.
    float lessThanEqualDistanceSqRadius(float dist2, float radius) const;

    /// Get the maximum distance from an object of the given radius that could
    /// result in this solid angle.  Effectively the inverse of fromCenterRadius.
    float maxDistance(float obj_radius) const;
protected:

    void clamp();

    float mSolidAngle;
}; // class SolidAngle

ELYSIA_FUNCTION_EXPORT std::ostream & operator<<(std::ostream & os, const Elysia::SolidAngle& sa);

} // namespace Elysia

#endif //_ELYSIA_SOLID_ANGLE_HPP_
