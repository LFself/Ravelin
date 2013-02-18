/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#include <iomanip>
#include <cstring>
#include <list>
#include <cmath>
#include <iostream>
#include <Ravelin/cblas.h>
#include <Ravelin/Constants.h>
#include <Ravelin/VectorNf.h>
#include <Ravelin/SharedVectorNf.h>
#include <Ravelin/SharedMatrixNf.h>

using namespace Ravelin;
using boost::shared_array;
using std::vector;

#define REAL float
#define MATRIXN MatrixNf
#define MATRIX3 Matrix3f
#define SHAREDVECTORN SharedVectorNf
#define SHAREDMATRIXN SharedMatrixNf
#define VECTORN VectorNf
#define VECTOR3 Vector3f
#define ITERATOR fIterator
#define CONST_ITERATOR fIterator_const

#include "SharedMatrixN.cpp"

#undef REAL
#undef MATRIXN
#undef MATRIX3
#undef SHAREDVECTORN
#undef SHAREDMATRIXN
#undef VECTORN
#undef VECTOR3
#undef ITERATOR
#undef CONST_ITERATOR

