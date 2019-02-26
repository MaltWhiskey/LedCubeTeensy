#include "Util.h"
#include <math.h>
/*----------------------------------------------------------------------------------------------
 * RANDOM CLASS
 *----------------------------------------------------------------------------------------------
 *
 */
float NoiseGenerator::nextRandom(float min, float max) {
  return min + random(1UL << 31) * (double)(max - min) / (1UL <<31);
}

float NoiseGenerator::nextGaussian(float mean, float stdev, int range) {
  float gauss;
  do {
	gauss = nextGaussian(mean, stdev);
  }
  while( (gauss > range*stdev+mean) || (gauss < -range*stdev+mean));
  return gauss;
}

float NoiseGenerator::nextGaussian(float mean, float stdev) {
  if(hasSpare) {
	hasSpare = false;
	return mean + stdev * spare;
  }
  hasSpare = true;
  float u, v, s;
  do {
	u = (rand() / ((float) RAND_MAX)) * 2.0 - 1.0;
	v = (rand() / ((float) RAND_MAX)) * 2.0 - 1.0;
	s = u * u + v * v;
  }
  while( (s >= 1.0) || (s == 0.0) );

  s = sqrt(-2.0 * log(s) / s);
  spare = v * s;
  return mean + stdev * u * s;
}
/*----------------------------------------------------------------------------------------------
 * TIMER CLASS
 *----------------------------------------------------------------------------------------------
 *
 */
Timer::Timer() {
  m_startTime=0;
}
Timer::Timer(float alarm) {
  operator=(alarm);
}
void Timer::operator=(float alarm) {
  m_startTime = 0;
  m_alarm = alarm;
}
int Timer::ticks() {
  m_currentTime = micros();
  if(m_startTime==0) {
    m_ticks = 0;
	m_startTime = m_currentTime;
	m_lastTime = m_currentTime;
  }
  m_deltaTime = m_currentTime-m_lastTime;
  m_runTime = m_currentTime-m_startTime;
  m_lastTime = m_currentTime;

  if(m_alarm > 0) {
	unsigned int threshold = m_runTime/(m_alarm * 1000000.0f);
	if(m_ticks < threshold) {
	  m_ticks  = threshold;
  	  return m_ticks;
	}
  }
  return 0;
}

bool Timer::expired() {
  ticks(); return (m_ticks!=0);
}
/*----------------------------------------------------------------------------------------------
 * OBJECT CLASS
 *----------------------------------------------------------------------------------------------
 * x,y,z are floats 8.9 is inside the cube even though width is 8 for the cube [0]...[8]
 * Because 8.9 gets truncated to 8 after casting to int. DON'T ROUND the values for x,y and z.
 * For example (int)x is 0 if x >= 0 and x < 1
 *
 * Delta Time (float dt) is in seconds
 */
void Object::move(float dt) {
  position = position + velocity*dt;
  velocity = velocity + gravity*dt;
}
// 3D triangle equation to determine if a particle is within a spherical radius
bool Object::hit(float x, float y, float z, float radius) const {
  return((x-position.x) * (x-position.x) +
	     (y-position.y) * (y-position.y) +
	     (z-position.z) * (z-position.z) <= radius * radius);
}
void Object::bounce(float dt, float vy, int width, int height, int depth) {
  Vector3 t = position + velocity * dt;
  if(t.x > width-1  || t.x < 0) velocity.x=-velocity.x;
  if(t.y > height-1 || t.y < 0) velocity.y=-velocity.y;
  if(t.z > depth-1  || t.z < 0) velocity.z=-velocity.z;
  if(t.y > height-1) velocity.y=-vy;
  position = position + velocity*dt;
}
void Object::drag(float dt, float n) {
  velocity = velocity * pow(n, dt);
}
// 0 .. 8 are inside, but 8.99 is also inside after truncating
bool Object::inside(int width, int height, int depth) {
  return (position.x < width && position.x >= 0) &&
		 (position.y < height && position.y >= 0) &&
		 (position.z < depth && position.z >= 0);
}
