/****************************************************************************
 * Copyright 2013 Evan Drumwright
 * This library is distributed under the terms of the GNU Lesser General Public 
 * License (found in COPYING).
 ****************************************************************************/

#ifndef POSE
#error This class is not to be included by the user directly. Use Posed.h or Posef.h instead. 
#endif

class AANGLE;
class MATRIX3;

/// A rigid body pose 
class POSE : public boost::enable_shared_from_this<POSE>
{
  public:
    POSE();
    POSE(const POSE& source) { operator=(source); }
    POSE(const AANGLE& a);
    POSE(const MATRIX3& m);
    POSE(const QUAT& q);
    POSE(const AANGLE& a, const VECTOR3& v);
    POSE(const MATRIX3& m, const VECTOR3& v);
    POSE(const QUAT& q, const VECTOR3& v);
    POSE(const VECTOR3& v);
    static POSE identity() { POSE T; T.set_identity(); return T; }
    static POSE interpolate(const POSE& m1, const POSE& m2, REAL t);
    VECTOR3 mult_point(const VECTOR3& v) const;
    VECTOR3 mult_vector(const VECTOR3& v) const;
    VECTOR3 inverse_mult_point(const VECTOR3& v) const;
    VECTOR3 inverse_mult_vector(const VECTOR3& v) const;
    WRENCH transform(boost::shared_ptr<POSE> p, const WRENCH& w) const;
    TWIST transform(boost::shared_ptr<POSE> p, const TWIST& t) const;
    void set_relative_pose(boost::shared_ptr<POSE> p); 
    SPATIAL_RB_INERTIA transform(boost::shared_ptr<POSE> p, const SPATIAL_RB_INERTIA& j) const;
    SPATIAL_AB_INERTIA transform(boost::shared_ptr<POSE> p, const SPATIAL_AB_INERTIA& j) const;
    POSE& set_identity();
    POSE& invert();
    POSE inverse() const { return inverse(*this); }
    static POSE inverse(const POSE& m);
    POSE& set(const AANGLE& a);
    POSE& set(const MATRIX3& m);
    POSE& set(const QUAT& q);
    POSE& set(const AANGLE& a, const VECTOR3& v);
    POSE& set(const MATRIX3& m, const VECTOR3& v);
    POSE& set(const QUAT& q, const VECTOR3& v);
    POSE& operator=(const POSE& source);
    POSE operator*(const POSE& m) const;

    /// the orientation of the pose frame
    QUAT q;

    /// the position of the pose frame
    VECTOR3 x;

    /// the pose that *this* pose is relative to (if any)
    boost::shared_ptr<POSE> rpose; 

  private:
    std::pair<QUAT, VECTOR3> calc_transform(boost::shared_ptr<const POSE> p) const;
    bool is_common(boost::shared_ptr<const POSE> p, unsigned& i) const;
}; // end class

std::ostream& operator<<(std::ostream& out, const POSE& m);

