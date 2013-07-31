/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

/// Constructs a zero vector
SVECTOR6::SVECTOR6()
{
  _data[0] = _data[1] = _data[2] = 0.0;
  _data[3] = _data[4] = _data[5] = 0.0;
}

/// Constructs a zero vector relative to the given pose
SVECTOR6::SVECTOR6(boost::shared_ptr<const POSE3> pose) 
{ 
  _data[0] = _data[1] = _data[2] = 0.0;
  _data[3] = _data[4] = _data[5] = 0.0;
  this->pose = pose; 
}

/// Constructs this vector with the given values
SVECTOR6::SVECTOR6(REAL x, REAL y, REAL z, REAL a, REAL b, REAL c)
{
  _data[0] = x;
  _data[1] = y;
  _data[2] = z;
  _data[3] = a;
  _data[4] = b;
  _data[5] = c;
}

/// Constructs this vector with the given values
SVECTOR6::SVECTOR6(REAL x, REAL y, REAL z, REAL a, REAL b, REAL c, boost::shared_ptr<const POSE3> pose)
{
  _data[0] = x;
  _data[1] = y;
  _data[2] = z;
  _data[3] = a;
  _data[4] = b;
  _data[5] = c;
  this->pose = pose;
}

/// Constructs this vector from the given array
/**
 * \param array a 6-dimensional (or larger) array
 */
SVECTOR6::SVECTOR6(const REAL* array)
{
  for (unsigned i=0; i< 6; i++)
    _data[i] = array[i];
}

/// Constructs this vector from the given array
/**
 * \param array a 6-dimensional (or larger) array
 */
SVECTOR6::SVECTOR6(const REAL* array, boost::shared_ptr<const POSE3> pose)
{
  for (unsigned i=0; i< 6; i++)
    _data[i] = array[i];
  this->pose = pose;
}

/// Constructs the given spatial vector with given upper and lower components
SVECTOR6::SVECTOR6(const VECTOR3& upper, const VECTOR3& lower)
{
  set_upper(upper);
  set_lower(lower);
}

/// Constructs the given spatial vector with given upper and lower components
SVECTOR6::SVECTOR6(const VECTOR3& upper, const VECTOR3& lower, boost::shared_ptr<const POSE3> pose)
{
  set_upper(upper);
  set_lower(lower);
  this->pose = pose;
}

/// Gets an iterator to the beginning of the data
CONST_ITERATOR SVECTOR6::begin() const
{
  CONST_ITERATOR i;
  i._count = 0;
  i._sz = 6;
  i._ld = 6;
  i._rows = i._sz;
  i._columns = 1;
  i._data_start = i._current_data  = _data;
  return i;
}

/// Gets an iterator to the end of the data
CONST_ITERATOR SVECTOR6::end() const
{
  CONST_ITERATOR i;
  i._count = 6;
  i._sz = 6;
  i._ld = 6;
  i._rows = i._sz;
  i._columns = 1;
  i._data_start = _data;
  i._current_data  = data() + i._sz;
  return i;
}

/// Gets an iterator to the beginning of the data
ITERATOR SVECTOR6::begin()
{
  ITERATOR i;
  i._count = 0;
  i._sz = 6;
  i._ld = 6;
  i._rows = i._sz;
  i._columns = 1;
  i._data_start = i._current_data  = _data;
  return i;
}

/// Gets an iterator to the end of the data
ITERATOR SVECTOR6::end()
{
  ITERATOR i;
  i._count = 6;
  i._sz = 6;
  i._ld = 6;
  i._rows = i._sz;
  i._columns = 1;
  i._data_start = _data;
  i._current_data  = data() + i._sz;
  return i;
}

/// Gets the lower 3-dimensional vector
VECTOR3 SVECTOR6::get_lower() const
{
  return VECTOR3(_data[3], _data[4], _data[5], pose);
}

/// Gets the upper 3-dimensional vector
VECTOR3 SVECTOR6::get_upper() const
{
  return VECTOR3(_data[0], _data[1], _data[2], pose);
}
 
/// Sets the lower 3-dimensional vector
void SVECTOR6::set_lower(const VECTOR3& lower)
{
  _data[3] = lower[0];
  _data[4] = lower[1];
  _data[5] = lower[2];
}

/// Sets the upper 3-dimensional vector
void SVECTOR6::set_upper(const VECTOR3& upper)
{
  _data[0] = upper[0];
  _data[1] = upper[1];
  _data[2] = upper[2];
}

/// Copies this vector from another SVECTOR6
SVECTOR6& SVECTOR6::operator=(const SVECTOR6& v)
{
  pose = v.pose;
  _data[0] = v._data[0];
  _data[1] = v._data[1];
  _data[2] = v._data[2];
  _data[3] = v._data[3];
  _data[4] = v._data[4];
  _data[5] = v._data[5];
  return *this;
}

/// Multiplies this vector by a scalar in place
SVECTOR6& SVECTOR6::operator*=(REAL scalar)
{
  _data[0] *= scalar;
  _data[1] *= scalar;
  _data[2] *= scalar;
  _data[3] *= scalar;
  _data[4] *= scalar;
  _data[5] *= scalar;

  return *this;
}

/*
/// Template specialization for dot product
template <>
REAL SVECTOR6::dot(const SVECTOR6& v, const SFORCE& w)
{
  #ifndef NEXCEPT
  if (v.pose != w.pose)
    throw FrameException();
  #endif

  const REAL* d1 = v.data(); 
  const REAL* d2 = w.data(); 
  return d1[3]+d2[0] + d1[4]+d2[1] + d1[5]+d2[2]+ 
         d1[0]+d2[3] + d1[1]+d2[4] + d1[2]+d2[5];
}

/// Template specialization for dot product
template <>
REAL SVECTOR6::dot(const SVECTOR6& v, const SACCEL& t)
{
  #ifndef NEXCEPT
  if (v.pose != t.pose)
    throw FrameException();
  #endif

  const REAL* d1 = v.data(); 
  const REAL* d2 = t.data(); 
  return d1[3]+d2[0] + d1[4]+d2[1] + d1[5]+d2[2]+ 
         d1[0]+d2[3] + d1[1]+d2[4] + d1[2]+d2[5];
}

/// Template specialization for dot product
template <>
REAL SVECTOR6::dot(const SVECTOR6& v, const SVELOCITY& t)
{
  #ifndef NEXCEPT
  if (v.pose != t.pose)
    throw FrameException();
  #endif

  const REAL* d1 = v.data(); 
  const REAL* d2 = t.data(); 
  return d1[3]+d2[0] + d1[4]+d2[1] + d1[5]+d2[2]+ 
         d1[0]+d2[3] + d1[1]+d2[4] + d1[2]+d2[5];
}
*/

