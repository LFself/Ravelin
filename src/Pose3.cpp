/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

/// Default constructor
/**
 * Sets matrix to the identity matrix
 */
POSE3::POSE3(boost::shared_ptr<POSE3> relative_pose)
{
  set_identity();
  rpose = relative_pose;
}

/// Constructs a pose from a unit quaternion and translation vector
POSE3::POSE3(const QUAT& q, const ORIGIN3& v, boost::shared_ptr<POSE3> relative_pose)
{
  set(QUAT::normalize(q), v);
  rpose = relative_pose;
}

/// Constructs a pose from a unit quaternion (for rotation) and zero translation
POSE3::POSE3(const QUAT& q, boost::shared_ptr<POSE3> relative_pose)
{
  set(QUAT::normalize(q), ORIGIN3::zero());
  rpose = relative_pose;
}

/// Constructs a pose from a rotation matrix and translation vector
POSE3::POSE3(const MATRIX3& r, const ORIGIN3& v, boost::shared_ptr<POSE3> relative_pose) 
{
  set(r, v);
  rpose = relative_pose;
}

/// Constructs a pose from a rotation matrix and zero translation
POSE3::POSE3(const MATRIX3& r, boost::shared_ptr<POSE3> relative_pose)
{
  set(r, ORIGIN3::zero());
  rpose = relative_pose;
}

/// Constructs a pose from a axis-angle representation and a translation vector
POSE3::POSE3(const AANGLE& a, const ORIGIN3& v, boost::shared_ptr<POSE3> relative_pose)
{
  set(a, v);
  rpose = relative_pose;
}

/// Constructs a pose from a axis-angle representation (for rotation) and zero translation
POSE3::POSE3(const AANGLE& a, boost::shared_ptr<POSE3> relative_pose)
{
  set(a, ORIGIN3::zero());
  rpose = relative_pose;
}

/// Constructs a pose using identity orientation and a translation vector
POSE3::POSE3(const ORIGIN3& v, boost::shared_ptr<POSE3> relative_pose)
{
  set(QUAT::identity(), v);
  rpose = relative_pose;
}

POSE3& POSE3::operator=(const POSE3& p)
{
  q = p.q;
  x = p.x;
  rpose = p.rpose;
  return *this;
}

/// Determines whether two poses in 3D are relatively equivalent
bool POSE3::rel_equal(const POSE3& p1, const POSE3& p2, REAL tol)
{
  // verify relative frames are identity 
  if (p1.rpose != p2.rpose)
    throw FrameException();

  // check x components first
  if (!OPS::rel_equal(p1.x[0], p2.x[0], tol) || 
      !OPS::rel_equal(p1.x[1], p2.x[1], tol) ||
      !OPS::rel_equal(p1.x[2], p2.x[2], tol))
    return false;

  // now check rotation (it's more expensive)
  REAL angle = QUAT::calc_angle(p1.q, p2.q);
  return OPS::rel_equal(angle, (REAL) 0.0, tol);
}

/// Interpolates between two poses using spherical linear interpolation
/**
 * \param T1 the matrix to use when t=0
 * \param T2 the matrix to use when t=1
 * \param t a real value in the interval [0,1]
 * \return the interpolated transform
 */
POSE3 POSE3::interpolate(const POSE3& T1, const POSE3& T2, REAL t)
{
  // interpolate the positions
  ORIGIN3 x = T1.x*(1-t) + T2.x*t;

  // interpolate the rotations
  QUAT q = QUAT::slerp(T1.q, T2.q, t);

  // return the new matrix
  return POSE3(q, x);
}

// Sets the pose from an axis-angle representation and a translation vector
/**
 * \todo make this marginally faster by setting the matrix components directly (eliminates redundancy in setting non-rotation and non-translational components)
 */
POSE3& POSE3::set(const AANGLE& a, const ORIGIN3& v)
{
  this->q = a;
  this->x = v;
  return *this;
}

/// Sets the pose from a rotation matrix and translation vector
POSE3& POSE3::set(const MATRIX3& m, const ORIGIN3& v)
{
  q = m;
  x = v;
  return *this;
}

/// Sets the pose from a unit quaternion and translation vector
POSE3& POSE3::set(const QUAT& q, const ORIGIN3& v)
{
  this->q = q;
  this->x = v;
  return *this;
}

/// Sets this pose from a unit quaternion only (translation will be zero'd) 
POSE3& POSE3::set(const QUAT& q)
{
  this->q = q;
  this->x = ORIGIN3::zero();
  return *this;
}

/// Sets this pose from a axis-angle object only (translation will be zero'd) 
POSE3& POSE3::set(const AANGLE& a)
{
  this->q = a;
  this->x = ORIGIN3::zero();
  return *this;
}

/// Sets this pose from a 3x3 rotation matrix only (translation will be zero'd) 
POSE3& POSE3::set(const MATRIX3& m)
{
  this->q = m;
  this->x = ORIGIN3::zero();
  return *this;
}

/// Sets this matrix to identity
POSE3& POSE3::set_identity()
{
  this->q = QUAT::identity();
  this->x = ORIGIN3::zero();
  return *this;
}

/// method for inverting a pose in place
POSE3& POSE3::invert()
{
  // get the new rotation 
  q.invert();

  // determine the new translation
  x = q * (-x);

  return *this;
}

/// Special method for inverting a pose
POSE3 POSE3::invert(const POSE3& p)
{
  return POSE3(p).invert();
}

/// Multiplies this pose by another
POSE3 POSE3::operator*(const POSE3& p) const
{
  #ifndef NEXCEPT
  if (rpose != p.rpose)
    throw FrameException();
  #endif
  return POSE3(q * p.q, q*p.x + x);
}

/// Transforms a vector from one pose to another 
VECTOR3 POSE3::transform(const VECTOR3& v) const
{
  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (v.pose != pose)
    throw FrameException();
  #endif

  VECTOR3 result = q * v;
  result.pose = rpose;
  return result;
}

/// Transforms a vector from one pose to another 
VECTOR3 POSE3::inverse_transform(const VECTOR3& v) const
{
  VECTOR3 result = QUAT::invert(q) * v;

  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (v.pose != rpose)
    throw FrameException();
  result.pose = boost::const_pointer_cast<POSE3>(pose);
  #endif

  return result;
}

/// Transforms a point from one pose to another 
POINT3 POSE3::transform(const POINT3& p) const
{
  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (p.pose != pose)
    throw FrameException();
  #endif

  POINT3 result = q * p + x;
  result.pose = rpose;
  return result;
}

/// Transforms a point from one pose to another 
POINT3 POSE3::inverse_transform(const POINT3& p) const
{
  POINT3 result = QUAT::invert(q) * (p - x);

  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (p.pose != rpose)
    throw FrameException();
  result.pose = boost::const_pointer_cast<POSE3>(pose);
  #endif

  return result;
}

/// Transforms a wrench from one pose to another 
WRENCH POSE3::transform(const WRENCH& w) const
{
  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (w.pose != pose)
    throw FrameException();
  #endif

  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, false);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // get the components of w
  VECTOR3 top = w.get_force();
  VECTOR3 bottom = w.get_torque();

  // do the calculations
  VECTOR3 Etop = E * top;
  VECTOR3 cross = VECTOR3::cross(r, Etop);
  WRENCH result(Etop, (E * bottom) - cross);
  result.pose = rpose;

  return result;
}

/// Transforms a point from one pose to another 
WRENCH POSE3::inverse_transform(const WRENCH& w) const
{
  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, true);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // get the components of w
  VECTOR3 top = w.get_force();
  VECTOR3 bottom = w.get_torque();

  // do the calculations
  VECTOR3 Etop = E * top;
  VECTOR3 cross = VECTOR3::cross(r, Etop);
  WRENCH result(Etop, (E * bottom) - cross);

  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (w.pose != rpose)
    throw FrameException();
  result.pose = boost::const_pointer_cast<POSE3>(pose);
  #endif

  return result;
}

/// Transforms a twist from one pose to another 
TWIST POSE3::transform(const TWIST& t) const
{
  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (t.pose != pose)
    throw FrameException();
  #endif

  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, false);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // get the components of t 
  VECTOR3 top = t.get_angular();
  VECTOR3 bottom = t.get_linear();

  // do the calculations
  VECTOR3 Etop = E * top;
  VECTOR3 cross = VECTOR3::cross(r, Etop);
  TWIST result(Etop, (E * bottom) - cross);
  result.pose = rpose;

  return result;
}

/// Transforms a twist from one pose to another 
TWIST POSE3::inverse_transform(const TWIST& t) const
{
  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, true);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // get the components of t 
  VECTOR3 top = t.get_angular();
  VECTOR3 bottom = t.get_linear();

  // do the calculations
  VECTOR3 Etop = E * top;
  VECTOR3 cross = VECTOR3::cross(r, Etop);
  TWIST result(Etop, (E * bottom) - cross);

  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (t.pose != rpose)
    throw FrameException();
  result.pose = boost::const_pointer_cast<POSE3>(pose);
  #endif

  return result;
}

/// Transforms a rigid body inertia from one pose to another 
SPATIAL_RB_INERTIA POSE3::transform(const SPATIAL_RB_INERTIA& J) const
{
  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (J.pose != pose)
    throw FrameException();
  #endif

  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, false);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // precompute some things
  VECTOR3 mr = r * J.m;
  MATRIX3 rx = MATRIX3::skew_symmetric(r);
  MATRIX3 hx = MATRIX3::skew_symmetric(J.h);
  MATRIX3 mrxrx = rx * MATRIX3::skew_symmetric(mr);  
  MATRIX3 EhxETrx = E * hx * ET * rx;

  // setup the new inertia
  SPATIAL_RB_INERTIA Jx;
  Jx.m = J.m;
  Jx.J = EhxETrx + MATRIX3::transpose(EhxETrx) + (E*J.J*ET) - mrxrx; 
  Jx.h = E * J.h - mr;
  Jx.pose = rpose;

  return Jx;
}

/// Transforms a rigid body inertia from one pose to another 
SPATIAL_RB_INERTIA POSE3::inverse_transform(const SPATIAL_RB_INERTIA& J) const
{
  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, true);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // precompute some things
  VECTOR3 mr = r * J.m;
  MATRIX3 rx = MATRIX3::skew_symmetric(r);
  MATRIX3 hx = MATRIX3::skew_symmetric(J.h);
  MATRIX3 mrxrx = rx * MATRIX3::skew_symmetric(mr);  
  MATRIX3 EhxETrx = E * hx * ET * rx;

  // setup the new inertia
  SPATIAL_RB_INERTIA Jx;
  Jx.m = J.m;
  Jx.J = EhxETrx + MATRIX3::transpose(EhxETrx) + (E*J.J*ET) - mrxrx; 
  Jx.h = E * J.h - mr;

  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (J.pose != rpose)
    throw FrameException();
  Jx.pose = boost::const_pointer_cast<POSE3>(pose);
  #endif

  return Jx;
}

/// Transforms an articulated body inertia from one pose to another 
SPATIAL_AB_INERTIA POSE3::transform(const SPATIAL_AB_INERTIA& J) const
{
  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (J.pose != pose)
    throw FrameException();
  #endif

  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, false);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // precompute some things we'll need
  MATRIX3 rx = MATRIX3::skew_symmetric(r);
  MATRIX3 HT = MATRIX3::transpose(J.H);
  MATRIX3 EJET = E * J.J * ET;
  MATRIX3 rx_E_HT_ET = rx*E*HT*ET;
  MATRIX3 EHET = E * J.H * ET;
  MATRIX3 EMET = E * J.M * ET;
  MATRIX3 rxEMET = rx * EMET;

  SPATIAL_AB_INERTIA result;
  result.M = EMET;
  result.H = EHET - rxEMET;
  result.J = EJET - rx_E_HT_ET + ((EHET - rxEMET) * rx); 
  result.pose = rpose;

  return result;
}

/// Transforms an articulated body inertia from one pose to another 
SPATIAL_AB_INERTIA POSE3::inverse_transform(const SPATIAL_AB_INERTIA& J) const
{
  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(r, E, true);
  const MATRIX3 ET = MATRIX3::transpose(E);

  // precompute some things we'll need
  MATRIX3 rx = MATRIX3::skew_symmetric(r);
  MATRIX3 HT = MATRIX3::transpose(J.H);
  MATRIX3 EJET = E * J.J * ET;
  MATRIX3 rx_E_HT_ET = rx*E*HT*ET;
  MATRIX3 EHET = E * J.H * ET;
  MATRIX3 EMET = E * J.M * ET;
  MATRIX3 rxEMET = rx * EMET;

  // compute the result
  SPATIAL_AB_INERTIA result;
  result.M = EMET;
  result.H = EHET - rxEMET;
  result.J = EJET - rx_E_HT_ET + ((EHET - rxEMET) * rx); 

  #ifndef NEXCEPT
  boost::shared_ptr<const POSE3> pose;
  try
  {
    pose = shared_from_this();
  }
  catch (boost::bad_weak_ptr e)
  {
    std::cerr << "Pose3 can't check frames when allocated on the stack! Allocate on heap or disable debugging exceptions." << std::endl;
  }
  if (J.pose != rpose)
    throw FrameException();
  result.pose = boost::const_pointer_cast<POSE3>(pose);
  #endif

  return result;
}

/// Gets r and E from the current pose only
void POSE3::get_r_E(VECTOR3& r, MATRIX3& E, bool inverse) const
{
  if (!inverse)
  {
    E = q;
    r = -x;
  }
  else
  {
    E = QUAT::invert(q);
    r = E * x;
  } 
}

/// Applies this pose to a vector 
VECTOR3 POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const VECTOR3& v) 
{
  #ifndef NEXCEPT
  if (source != v.pose)
    throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  return Tx.transform(v);
}

/// Transforms a point from one pose to another 
POINT3 POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const POINT3& point)
{
  #ifndef NEXCEPT
  if (source != point.pose)
    throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // do the transform
  return Tx.transform(point);
}

/// Transforms a spatial articulated body inertia 
SPATIAL_AB_INERTIA POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const SPATIAL_AB_INERTIA& m)
{
  #ifndef NEXCEPT
  if (source != m.pose)
    throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // setup r and E
  VECTOR3 r = Tx.x;
  const MATRIX3 E = Tx.q;
  const MATRIX3 ET = MATRIX3::transpose(E);

  // precompute some things we'll need
  MATRIX3 rx = MATRIX3::skew_symmetric(r);
  MATRIX3 HT = MATRIX3::transpose(m.H);
  MATRIX3 EJET = E * m.J * ET;
  MATRIX3 rx_E_HT_ET = rx*E*HT*ET;
  MATRIX3 EHET = E * m.H * ET;
  MATRIX3 EMET = E * m.M * ET;
  MATRIX3 rxEMET = rx * EMET;

  SPATIAL_AB_INERTIA result;
  result.pose = boost::const_pointer_cast<POSE3>(target);
  result.M = EMET;
  result.H = EHET - rxEMET;
  result.J = EJET - rx_E_HT_ET + ((EHET - rxEMET) * rx); 
  return result;
}

/// Transforms a vector of wrenches 
std::vector<WRENCH>& POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const std::vector<WRENCH>& w, std::vector<WRENCH>& result)
{
  // look for empty vector (easy case)
  if (w.empty())
  {
    result.clear();
    return result;
  }

  #ifndef NEXCEPT
  for (unsigned i=0; i< w.size(); i++)
    if (source != w[i].pose)
      throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // setup r and E
  const ORIGIN3& r = Tx.x;
  const MATRIX3 E = Tx.q;

  // resize the result vector
  result.resize(w.size());

  // look over all wrenches
  for (unsigned i=0; i< w.size(); i++)
  {
    // get the components of w[i] 
    VECTOR3 top = w[i].get_force();
    VECTOR3 bottom = w[i].get_torque();

    // do the calculations
    VECTOR3 Etop = E * top;
    VECTOR3 cross = VECTOR3::cross(r, Etop);
    result[i] = WRENCH(Etop, (E * bottom) - cross);
    result[i].pose = boost::const_pointer_cast<POSE3>(target);
  }

  return result;
}

/// Transforms the wrench 
WRENCH POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const WRENCH& v)
{
  #ifndef NEXCEPT
  if (source != v.pose)
    throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // setup r and E
  const ORIGIN3& r = Tx.x;
  const MATRIX3 E = Tx.q;

  // get the components of v
  VECTOR3 top = v.get_force();
  VECTOR3 bottom = v.get_torque();

  // do the calculations
  VECTOR3 Etop = E * top;
  VECTOR3 cross = VECTOR3::cross(r, Etop);
  WRENCH result(Etop, (E * bottom) - cross);
  result.pose = boost::const_pointer_cast<POSE3>(target);
  return result;
}

/// Transforms the twist 
TWIST POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const TWIST& t)
{
  #ifndef NEXCEPT
  if (source != t.pose)
    throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // setup r and E
  const ORIGIN3& r = Tx.x;
  const MATRIX3 E = Tx.q;

  // get the components of t 
  VECTOR3 top = t.get_angular();
  VECTOR3 bottom = t.get_linear();

  // do the calculations
  VECTOR3 Etop = E * top;
  VECTOR3 cross = VECTOR3::cross(r, Etop);
  TWIST result(Etop, (E * bottom) - cross);
  result.pose = boost::const_pointer_cast<POSE3>(target);
  return result;
}

/// Transforms a vector of twists 
std::vector<TWIST>& POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const std::vector<TWIST>& t, std::vector<TWIST>& result)
{
  // look for empty vector (easy case)
  if (t.empty())
  {
    result.clear();
    return result;
  }

  #ifndef NEXCEPT
  for (unsigned i=0; i< t.size(); i++)
    if (source != t[i].pose)
      throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // setup r and E
  const ORIGIN3& r = Tx.x;
  const MATRIX3 E = Tx.q;

  // resize the result vector
  result.resize(t.size());

  // look over all wrenches
  for (unsigned i=0; i< t.size(); i++)
  {
    // get the components of t[i] 
    VECTOR3 top = t[i].get_angular();
    VECTOR3 bottom = t[i].get_linear();

    // do the calculations
    VECTOR3 Etop = E * top;
    VECTOR3 cross = VECTOR3::cross(r, Etop);
    result[i] = TWIST(Etop, (E * bottom) - cross);
    result[i].pose = boost::const_pointer_cast<POSE3>(target);
  }

  return result;
}

/// Transforms a spatial RB inertia to the given pose
SPATIAL_RB_INERTIA POSE3::transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target, const SPATIAL_RB_INERTIA& J)
{
  #ifndef NEXCEPT
  if (source != J.pose)
    throw FrameException();
  #endif

  // compute the relative transform
  TRANSFORM3 Tx = calc_transform(source, target);

  // setup r and E
  const ORIGIN3& r = Tx.x;
  const MATRIX3 E = Tx.q;
  const MATRIX3 ET = MATRIX3::transpose(E);

  // precompute some things
  VECTOR3 mr = r * J.m;
  MATRIX3 rx = MATRIX3::skew_symmetric(r);
  MATRIX3 hx = MATRIX3::skew_symmetric(J.h);
  MATRIX3 mrxrx = rx * MATRIX3::skew_symmetric(mr);  
  MATRIX3 EhxETrx = E * hx * ET * rx;

  // setup the new inertia
  SPATIAL_RB_INERTIA Jx;
  Jx.pose = boost::const_pointer_cast<POSE3>(target);
  Jx.m = J.m;
  Jx.J = EhxETrx + MATRIX3::transpose(EhxETrx) + (E*J.J*ET) - mrxrx; 
  Jx.h = E * J.h - mr;
  return Jx;
}

/// Determines whether pose p exists in the chain of relative poses (and, if so, how far down the chain)
bool POSE3::is_common(boost::shared_ptr<const POSE3> x, boost::shared_ptr<const POSE3> p, unsigned& i)
{
  // reset i
  i = 0;

  while (x)
  {
    if (x == p)
      return true;
    x = x->rpose;
    i++;
  }

  return false;
}

/// Computes the relative transformation from this pose to another
TRANSFORM3 POSE3::calc_transform(boost::shared_ptr<const POSE3> source, boost::shared_ptr<const POSE3> target)
{
  boost::shared_ptr<const POSE3> r, s; 
  TRANSFORM3 result;

  // setup the source and targets
  result.source = source;
  result.target = target;

  // check for special case: no transformation 
  if (source == target)
  {
    result.q.set_identity();
    result.x.set_zero();
    return result;
  }

  // check for special case: transformation to global frame
  if (!target)
  {
    // combine transforms from this to i: this will give aTl
    result.q = source->q;
    result.x = source->x;
    s = source;
    while (s)
    {
      s = s->rpose;
      if (!s)
        break;
      result.x = s->x + s->q * result.x;
      result.q = s->q * result.q;
    }

    // setup the transform 
    return result; 
  }

  // check for special case: transformation from global frame
  if (!source)
  {
    // combine transforms from target to q
    result.q = target->q;
    result.x = target->x;
    r = target;
    while (r)
    {
      r = r->rpose;
      if (!r)
        break;
      result.x = r->x + r->q * result.x; 
      result.q = r->q * result.q;
    }

    // compute the inverse pose of the right 
    result.q = QUAT::invert(result.q);
    result.x = result.q * (-result.x);
    return result;
  }

  // if both transforms are defined relative to the same frame, this is easy
  if (source->rpose == target->rpose)
  {
    // compute the inverse pose of p 
    result.q = QUAT::invert(target->q) * source->q;
    result.x = result.q * (source->x - target->x);
    return result;
  }
  else
  {
    // search for the common link
    unsigned i = std::numeric_limits<unsigned>::max();
    r = target;
    while (true)
    {
      if (is_common(source, r, i))
        break;
      else
      {
        assert(r);
        r = r->rpose;
      } 
    } 
    
     // combine transforms from this to i: this will give aTl
    QUAT left_q = source->q;
    ORIGIN3 left_x = source->x;
    s = source;
    for (unsigned j=0; j < i; j++)
    {
      s = s->rpose;
      left_x = s->x + s->q * left_x;
      left_q = s->q * left_q;
    }

    // combine transforms from target to q
    QUAT right_q = target->q;
    ORIGIN3 right_x = target->x;
    while (target != r)
    {
      target = target->rpose;
      right_x = target->x + target->q * right_x; 
      right_q = target->q * right_q;
    }

    // compute the inverse pose of the right 
    QUAT inv_right_q = QUAT::invert(right_q);

    // multiply the inverse pose of p by this 
    result.q = inv_right_q * left_q;      
    result.x = inv_right_q * (left_x - right_x);
    return result;
  }
}

/// Outputs this matrix to the stream
std::ostream& Ravelin::operator<<(std::ostream& out, const POSE3& m)
{
  out << "q: " << m.q << " " << m.x << std::endl;
   
  return out;
}
