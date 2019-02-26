#ifndef QUATERNION_H_
#define QUATERNION_H_

class Vector3 {
public:
  float x,y,z;
public:
  // constructors
  Vector3();
  Vector3(float x, float y, float z);
  Vector3(const Vector3& v);
  // copy assignment
  Vector3& operator=(const Vector3& v);

  // moving
  Vector3 operator+(const Vector3& v) const;
  Vector3 operator-(const Vector3& v) const;
  void operator+=(const Vector3& v);
  void operator-=(const Vector3& v);
  // negate
  Vector3 operator-() const;

  // scaling
  Vector3 operator*(float s) const;
  Vector3 operator/(float s) const;
  void operator*=(float s);
  void operator/=(float s);

  // cross product
  Vector3 cross(const Vector3& v) const;
  Vector3 operator*(const Vector3& v) const;
  void operator*=(const Vector3& v);

  // dot product
  float dot(const Vector3& v) const;
  float operator%(const Vector3& v) const;

  // unit vector
  void normalize();
  Vector3 normalized() const;
  // magnitude or length of the vector
  float magnitude() const;
  float norm() const;

  // rotate v by this axis (vector) and angle
  void rotate(float angle, Vector3& v) const;
  Vector3 rotated(float angle, const Vector3& v) const;

  // test if vector is inside object space
  bool inside(int w, int h, int d);
};


class Quaternion {
public:
  float w;
  Vector3 v;
public:
  // constructors
  Quaternion(float a, const Vector3& v);
  Quaternion();
  Quaternion(const Quaternion& q);
  // copy assignment
  Quaternion& operator=(const Quaternion& q);
  void convertAxisAngle();

  // moving (add subtract)
  Quaternion operator+(const Quaternion& q) const;
  Quaternion operator-(const Quaternion& q) const;
  void operator+=(const Quaternion& q);
  void operator-=(const Quaternion& q);

  // scaling (multiply divide by scalar)
  Quaternion operator*(float s) const;
  Quaternion operator/(float s) const;

  void operator*=(float s);
  void operator/=(float s);

  // multiply quaternions
  Quaternion operator*(const Quaternion& q) const;
  Quaternion operator/(const Quaternion& q) const;
  void operator*=(const Quaternion& q);

  // dot product
  float dot(const Quaternion& q) const;
  float operator%(const Quaternion& q) const;

  // inverse
  void inverse();
  Quaternion inversed() const;
  // get conjugate (negative imaginary part)
  void conjugate();
  Quaternion conjugated() const;
  // unit quaternion
  void normalize();
  Quaternion normalized() const;
  // magnitude or length of the quaterion
  float magnitude() const;
  float norm() const;
  // rotate v by quaternion
  void rotate(Vector3& v) const;
  Vector3 rotated(const Vector3& v) const;
};
#endif
