#include <math.h>
#include <stdlib.h>
#include "Quaternion.h"
#define PI M_PI
#define M_1_360PI PI/360
#define M_1_180PI PI/180
/*----------------------------------------------------------------------------------------------
 * V3 CLASS
 *----------------------------------------------------------------------------------------------
 * A vector V in physics is an (angel, magnitude) magnitude is length and the angle
 * is usually represented by Greek letter theta.
 *
 * In game development a vector is stored as (x,y,z) representing a movement
 * in the Cartesian plane across the x, y and z axis.
 *
 * A point or the position of an object can also be represented by a vector by moving
 * from the origin to the coordinates (x,y,z)
 *
 * The length = sqrt(x*x + y*y + z*z)
 */
// constructors
Vector3::Vector3():x(0.0f),y(0.0f),z(0.0f) {};
Vector3::Vector3(float x_,float y_,float z_):x(x_),y(y_),z(z_) {};
Vector3::Vector3(const Vector3 &v):x(v.x),y(v.y),z(v.z) {};
// copy assignment (operator =)
Vector3& Vector3::operator=(const Vector3& v) {
  x=v.x; y=v.y; z=v.z;
  return *this;
}

// add, subtract (operator +, -, +=, -=)
Vector3 Vector3::operator+(const Vector3& v) const {
  return Vector3(x+v.x, y+v.y, z+v.z);
}
Vector3 Vector3::operator-(const Vector3& v) const {
  return Vector3(x-v.x, y-v.y, z-v.z);
}
void Vector3::operator+=(const Vector3& v) {
  x+=v.x; y+=v.y; z+=v.z;
}
void Vector3::operator-=(const Vector3& v) {
  x-=v.x; y-=v.y; z-=v.z;
}
// negative (operator -)
Vector3 Vector3::operator-() const {
  return Vector3(-x, -y, -z);
}

// multiply, divide by scalar (operator *, /, *=, /=)
Vector3 Vector3::operator*(float s) const {
  return Vector3(x*s, y*s, z*s);
}
Vector3 Vector3::operator/(float s) const {
  return Vector3(x/s, y/s, z/s);
}
void Vector3::operator*=(float s) {
  x*=s; y*=s; z*=s;
}
void Vector3::operator/=(float s) {
  x/=s; y/=s; z/=s;
}

// cross product (operator *, *=)
Vector3 Vector3::cross(const Vector3& v) const {
  return Vector3(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x);
}
Vector3 Vector3::operator*(const Vector3& v) const {
  return cross(v);
}
void Vector3::operator*=(const Vector3& v) {
  *this=cross(v);
}

// dot product (operator %)
float Vector3::dot(const Vector3& v) const {
  return x*v.x+y*v.y+z*v.z;
}
float Vector3::operator%(const Vector3& v) const {
  return dot(v);
}

// normalize
void Vector3::normalize() {
  *this/=magnitude();
}
Vector3 Vector3::normalized() const {
  return (*this)/magnitude();
}
// magnitude
float Vector3::magnitude() const {
  return sqrt(norm());
}
float Vector3::norm() const {
  return x*x+y*y+z*z;
}

// test if vector is inside object space
bool Vector3::inside(int width, int height, int depth) {
  return (x < width && x >= 0) &&
		 (y < height && y >= 0) &&
		 (z < depth && z >= 0);
}
// UNTESTED CODE!!!! TODO TEST THIS!!!
// rotate v by this vector (axis) and angle using Rodrigues formula
// Angle is in degree and is converted to radian by 2PI/360 * angle => PI/180 * angle
void Vector3::rotate(float angle, Vector3& v) const {
  float c = cosf(angle);
  float s = sinf(angle);
  // normalize this vector to get n hat
  Vector3 n = normalized();
  // (1-cos(0))(v.n)n + cos(0)v + sin(0)(n x v)
  v = n*((1-c)*n.dot(v)) + ((v*c) + n.cross(v)*s);
}
Vector3 Vector3::rotated(float angle, const Vector3& v) const {
  Vector3 v_ = v;
  rotate(angle, v_);
  return v_;
}

/*----------------------------------------------------------------------------------------------
 * Q4 CLASS (QUATERNION)
 *----------------------------------------------------------------------------------------------
 * A Quaternion is a complex number in the form  w + xi + yj + zk, where w, x, y, z are real
 * numbers and i, j, k are imaginary.
 *
 * In the implementation i,j and k are ignored, w is a scalar and x,y,z is a vector
 */
// constructors
Quaternion::Quaternion():w(0.0f),v(Vector3(0.0, 0.0, 0.0)){}
Quaternion::Quaternion(float w_, const Vector3& v_) {
  w=w_; v=v_;
}
Quaternion::Quaternion(const Quaternion& q):w(q.w),v(q.v){}
// copy assignment (operator =)
Quaternion& Quaternion::operator=(const Quaternion& q) {
  w=q.w; v=q.v;
  return *this;
}
// Makes a real quaternion from an angle and an axis stored in this 'fake' quaternion
// Angle is in degree and is converted to radian by 2PI/360 * angle => PI/180 * angle
// Angles are divided by 2 when using quaternions so back to PI/360 => M_1_360PI
void Quaternion::convertAxisAngle() {
  v.normalize();
  w*=M_1_360PI;
  v*=sinf(w);
  w=cosf(w);
}

// add, subtract (operator +, -, +=, -=)
Quaternion Quaternion::operator+(const Quaternion& q) const {
  return Quaternion(w+q.w, v+q.v);
}
Quaternion Quaternion::operator-(const Quaternion& q) const {
  return Quaternion(w-q.w, v-q.v);
}
void Quaternion::operator+=(const Quaternion& q) {
  w+=q.w; v+=q.v;
}
void Quaternion::operator-=(const Quaternion& q) {
  w-=q.w; v-=q.v;
}

// multiply, divide by scalar (operator *, /, *=, /=)
Quaternion Quaternion::operator*(float s) const {
  return Quaternion(w*s, v*s);
}
Quaternion Quaternion::operator/(float s) const {
  return Quaternion(w/s, v/s);
}
void Quaternion::operator*=(float s) {
  w*=s; v*=s;
}
void Quaternion::operator/=(float s) {
  w/=s; v/=s;
}

// multiply quaternions (operator *, /, *=)
Quaternion Quaternion::operator*(const Quaternion& q) const {
  Quaternion r;
  r.w = w*q.w - v.dot(q.v);
  r.v = v*q.w + q.v*w + v.cross(q.v);
  return r;
}
Quaternion Quaternion::operator/(const Quaternion& q) const {
    return ((*this)*q.inversed());
}
void Quaternion::operator*=(const Quaternion& q) {
  (*this)=operator*(q);
}

// dot product (operator %)
float Quaternion::dot(const Quaternion& q) const {
  return w*q.w + v.dot(q.v);
}
float Quaternion::operator%(const Quaternion& q) const {
  return dot(q);
}

// inverse
void Quaternion::inverse() {
  conjugate();
  *this*=1/norm();
}
Quaternion Quaternion::inversed() const {
  return conjugated()*(1/norm());
}
// conjugate
void Quaternion::conjugate() {
  v= -v;
}
Quaternion Quaternion::conjugated() const {
  return Quaternion(w, -v);
}
// normalize
void Quaternion::normalize() {
  *this/=magnitude();
}
Quaternion Quaternion::normalized() const {
  return (*this)/magnitude();
}
// magnitude
float Quaternion::magnitude() const {
  return sqrt(norm());
}
float Quaternion::norm() const {
  return w*w + v.dot(v);
}
// rotate v_ by this quaternion
void Quaternion::rotate(Vector3& v_) const {
  // creates a pure quaternion from a vector
  Quaternion p = Quaternion(0, v_);
  // multiply (p)(q)(pi)
  v_ = ((*this)*p*(*this).inversed()).v;
}
Vector3 Quaternion::rotated(const Vector3& v_) const {
  // creates a pure quaternion from a vector
  Quaternion p = Quaternion(0, v_);
  // multiply (p)(q)(pi)
  return ((*this)*p*(*this).inversed()).v;
}
