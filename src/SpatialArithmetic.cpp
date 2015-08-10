/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the Apache V2.0 
 * License (obtainable from http://www.apache.org/licenses/LICENSE-2.0).
 ****************************************************************************/

using std::vector;

/// Duplication of POSE3::get_r_E(.)
/// Gets r and E from a transform 
/**
 * \param T a transformation from frame A to frame B
 * \param r on return, the vector from A's origin to B's origin in A frame 
 * \param E rotates vectors in A's orientation to vectors in B's orientation 
 */ 
static void get_r_E(const TRANSFORM3& T, VECTOR3& r, MATRIX3& E)
{
  // x is the translation from frame A to frame B

  // p_a
  // bTa * p_a = bQa * p_a + bXa

  // note that x is translation from relative pose to this pose
  // q is rotation from vectors in this pose to relative pose 
  E = T.q;
  r = VECTOR3(E.transpose_mult(-T.x), T.source);
}

/// transforms a spatial acceleration using precomputation *without accounting for moving frames*
void SPARITH::transform_accel(boost::shared_ptr<const POSE3> target, const SACCEL& w, const VECTOR3& r, const MATRIX3& E, SACCEL& result)
{
  // get the components of w[i] 
  VECTOR3 top = w.get_upper();
  VECTOR3 bottom = w.get_lower();

  // do the calculations
  VECTOR3 Etop(E * ORIGIN3(top), target);
  VECTOR3 cross = VECTOR3::cross(r, top);
  result.set_upper(Etop);
  result.set_lower(VECTOR3(E * ORIGIN3(bottom - cross), target));
  result.pose = target;
} 

/// Special transformation of acceleration when moving aspect of pose accounted for elsewhere
SACCEL SPARITH::transform_accel(boost::shared_ptr<const POSE3> target, const SACCEL& a)
{
  SACCEL s;

  // NOTE: this is a duplication of the transform_spatial(.) function in
  // Ravelin::Pose3x
  // setup the source pose 
  boost::shared_ptr<const POSE3> source = a.pose;

  // quick check
  if (source == target)
  {
    s=a;
    return s;
  }

  // compute the relative transform
  TRANSFORM3 Tx = POSE3::calc_relative_pose(source, target);

  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(Tx, r, E);

  // get the components of a
  VECTOR3 top = a.get_upper();
  VECTOR3 bottom = a.get_lower();

  // do the calculations
  VECTOR3 Etop(E * ORIGIN3(top), target);
  VECTOR3 cross = VECTOR3::cross(r, top);
  s.set_upper(Etop);
  s.set_lower(VECTOR3(E * ORIGIN3(bottom - cross), target));
  s.pose = target;
  return s;
}

/// Special transformation of acceleration when moving aspect of pose accounted for elsewhere
std::vector<SACCEL>& SPARITH::transform_accel(boost::shared_ptr<const POSE3> target, const std::vector<SACCEL>& t, std::vector<SACCEL>& result)
{
  // NOTE: this is a duplication of the transform_spatial(.) function in
  // Ravelin::Pose3x

  // look for empty vector (easy case)
  if (t.empty())
  {
    result.clear();
    return result;
  }

  // setup the source pose
  boost::shared_ptr<const POSE3> source = t[0].pose; 

  #ifndef NEXCEPT
  for (unsigned i=1; i< t.size(); i++)
    if (source != t[i].pose)
      throw FrameException();
  #endif

  // quick check
  if (source == target)
    return (result = t);

  // compute the relative transform
  TRANSFORM3 Tx = POSE3::calc_relative_pose(source, target);

  // setup r and E
  VECTOR3 r;
  MATRIX3 E;
  get_r_E(Tx, r, E);

  // resize the result vector
  result.resize(t.size());

  // transform the individual vectors 
  for (unsigned i=0; i< t.size(); i++)
    transform_accel(target, t[i], r, E, result[i]);

  return result;
}

/// Concates a vector with a force to make a new vector
VECTORN& SPARITH::concat(const VECTORN& v, const SFORCE& w, VECTORN& result)
{
  const unsigned SPATIAL_DIM = 6;
  result.resize(v.size()+SPATIAL_DIM);
  result.set_sub_vec(0, v);
  result.set_sub_vec(v.size()+0, w.get_force());
  result.set_sub_vec(v.size()+3, w.get_torque());
  return result;
}

/// Concates a vector with a momentum to make a new vector
VECTORN& SPARITH::concat(const VECTORN& v, const SMOMENTUM& w, VECTORN& result)
{
  const unsigned SPATIAL_DIM = 6;
  result.resize(v.size()+SPATIAL_DIM);
  result.set_sub_vec(0, result);
  result.set_sub_vec(v.size()+0, w.get_linear());
  result.set_sub_vec(v.size()+3, w.get_angular());
  return result;
}

/// Multiplies a vector of spatial momenta by a vector 
VECTORN& SPARITH::mult(const vector<SMOMENTUM>& Is, const VECTORN& v, VECTORN& result)
{
  const unsigned SPATIAL_DIM = 6;

  if (Is.size() != v.rows())
    throw MissizeException();

  // setup the result
  result.set_zero(SPATIAL_DIM);

  // if the vector is empty, return now
  if (Is.empty())
    return result;

  // verify that all twists are in the same pose
  for (unsigned i=1; i< Is.size(); i++)
    if (Is[i].pose != Is[i-1].pose)
      throw FrameException(); 

  // finally, do the computation
  const REAL* vdata = v.data();
  REAL* rdata = result.data();
  for (unsigned j=0; j< SPATIAL_DIM; j++)
    for (unsigned i=0; i< Is.size(); i++)
    {
      const REAL* Isdata = Is[i].data();
      rdata[j] += Isdata[j]*vdata[i];
    }

  return result;
}

/// Multiplies a vector of spatial momenta by a matrix 
MATRIXN& SPARITH::mult(const vector<SMOMENTUM>& Is, const MATRIXN& m, MATRIXN& result)
{
  const unsigned SPATIAL_DIM = 6;

  if (Is.size() != m.rows())
    throw MissizeException();

  // setup the result
  result.set_zero(SPATIAL_DIM, m.columns());

  // if the vector is empty, return now
  if (Is.empty())
    return result;

  // verify that all twists are in the same pose
  for (unsigned i=1; i< Is.size(); i++)
    if (Is[i].pose != Is[i-1].pose)
      throw FrameException(); 

  // finally, do the computation
  REAL* rdata = result.data();
  for (unsigned k=0; k< m.columns(); k++)
  {
    const REAL* mdata = m.column(k).data();
    for (unsigned j=0; j< SPATIAL_DIM; j++)  // j is row index for Is
      for (unsigned i=0; i< Is.size(); i++)  // i is column index for Is
      {
        const REAL* Isdata = Is[i].data();
        rdata[k*result.rows()+j] += Isdata[j]*mdata[i];
      }
  }

  return result;
}

/// Multiplies a spatial inertia by a vector of spatial axes
MATRIXN& SPARITH::mult(const SPATIAL_AB_INERTIA& I, const std::vector<SVELOCITY>& s, MATRIXN& result)
{
  const unsigned SPATIAL_DIM = 6;

  // resize the result
  result.resize(SPATIAL_DIM, s.size());

  // compute the individual momenta
  for (unsigned i=0; i< s.size(); i++)
  {
    SMOMENTUM m = I.mult(s[i]);
    SHAREDVECTORN col = result.column(i);
    m.to_vector(col);
  } 

  return result;
}

/// Multiplies a spatial inertia by a vector of spatial axes
vector<SMOMENTUM>& SPARITH::mult(const SPATIAL_AB_INERTIA& I, const std::vector<SVELOCITY>& s, vector<SMOMENTUM>& result)
{
  const unsigned SPATIAL_DIM = 6;

  // resize the result
  result.resize(s.size());

  // compute the individual momenta
  for (unsigned i=0; i< s.size(); i++)
    result[i] = I.mult(s[i]);

  return result;
}

/// Multiplies a spatial inertia by a vector of spatial axes
MATRIXN& SPARITH::mult(const SPATIAL_RB_INERTIA& I, const std::vector<SVELOCITY>& s, MATRIXN& result)
{
  const unsigned SPATIAL_DIM = 6;

  // resize the result
  result.resize(SPATIAL_DIM, s.size());

  // compute the individual momenta
  for (unsigned i=0; i< s.size(); i++)
  {
    SMOMENTUM m = I.mult(s[i]);
    SHAREDVECTORN col = result.column(i);
    m.to_vector(col);
  } 

  return result;
}

/// Multiplies a spatial inertia by a vector of spatial axes
vector<SMOMENTUM>& SPARITH::mult(const SPATIAL_RB_INERTIA& I, const std::vector<SVELOCITY>& s, vector<SMOMENTUM>& result)
{
  const unsigned SPATIAL_DIM = 6;

  // resize the result
  result.resize(s.size());

  // compute the individual momenta
  for (unsigned i=0; i< s.size(); i++)
    result[i] = I.mult(s[i]);

  return result;
}

/// Multiplies a vector of spatial axes by a vector
SVELOCITY SPARITH::mult(const vector<SVELOCITY>& t, const VECTORN& v)
{
  const unsigned SPATIAL_DIM = 6;

  if (t.size() != v.size())
    throw MissizeException();

  // verify that the vector is not empty - we lose frame info!
  if (t.empty())
    throw std::runtime_error("loss of frame information");

  // setup the result
  SVELOCITY result = SVELOCITY::zero();

  // verify that all twists are in the same pose
  result.pose = t.front().pose;
  for (unsigned i=1; i< t.size(); i++)
    if (t[i].pose != result.pose)
      throw FrameException(); 

  // finally, do the computation
  const REAL* vdata = v.data();
  for (unsigned j=0; j< SPATIAL_DIM; j++)
    for (unsigned i=0; i< t.size(); i++)
      result[j] += t[i][j]*vdata[i];

  return result;
}

