#ifndef UTIL_H
#define UTIL_H
#include "Color.h"
#include "Quaternion.h"

class NoiseGenerator {
private:
  bool hasSpare = false;
  float spare;
public:
  float nextGaussian(float mean, float stdev);
  float nextGaussian(float mean, float stdev, int range);
  float nextRandom(float min, float max);
};

class Timer {
public:
  Timer();
  Timer(float);
  void operator=(float);
  int ticks();
  bool expired();
private:
  float m_alarm = 0;
  unsigned int m_ticks = 0;
  unsigned long m_startTime = 0;
  unsigned long m_lastTime = 0;
  unsigned long m_currentTime = 0;
  unsigned long m_deltaTime = 0;
  unsigned long m_runTime = 0;
};

class Object {
public:
  Vector3 position = Vector3(0,0,0);
  Vector3 velocity = Vector3(0,0,0);
  Vector3 gravity  = Vector3(0,0,0);
  ColorBlender cb;
public:
  void move(float dt);
  // determine is v is within radius of this
  bool hit(float x, float y, float z, float radius) const;
  // bounce object on top of display
  void bounce(float dt, float vy, int width, int height, int depth);
  // put some resistance on the moving of the object
  void drag(float dt, float n);
  // checks if coordinates are within the display area
  bool inside(int width, int height, int depth);
};
#endif
