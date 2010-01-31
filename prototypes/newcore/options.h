// Aqsis
// Copyright (C) 1997 - 2010, Paul C. Gregory
//
// Contact: pgregory@aqsis.org
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include <cfloat>

/// Renderer options.
struct Options
{
    int maxSplits;   ///< maximum number of splits before discarding a surface
    int xRes;        ///< image x-resolution
    int yRes;        ///< image y-resolution
    //Imath::V2i nsumSamples; ///< number of subsamples
    int gridSize;    ///< Desired grid side length.
    float clipNear;  ///< Near clipping plane (cam coords)
    float clipFar;   ///< Far clipping plane (cam coords)

    Options()
        : maxSplits(20),
        xRes(640),
        yRes(480),
        gridSize(16),
        clipNear(FLT_EPSILON),
        clipFar(FLT_MAX)
    { }
};

#endif // OPTIONS_H_INCLUDED
