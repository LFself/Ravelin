/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#ifndef _TWISTF_H
#define _TWISTF_H

#include <Moby/SVector6f.h>

namespace Moby {

#define REAL float
#define TWIST Wrenchf
#define SVECTOR6 SVector6f
#define VECTOR3 Vector3f

#include "Twist.h"

#undef REAL
#undef TWIST
#undef SVECTOR6
#undef VECTOR3

} // end namespace

#endif

