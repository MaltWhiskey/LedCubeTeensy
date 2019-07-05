#ifndef ANIMATION_H
#define ANIMATION_H
#include "Util.h"

class Animation {
public:
  virtual ~Animation();
  // run animation with respect to size
  void animate(int width, int height, int depth);
  // test if animation is still running
  bool running();
  // restarts animation next time animate is called
  void restart();
protected:
  // drawing method needs to be overridden
  virtual void draw(float dt) = 0;
  // init method needs to be overridden
  virtual void init() = 0;
private:
  // time of first frame of this animation
  unsigned long m_startTime = 0;
  // time of last frame of this animation
  unsigned long m_lastTime = 0;
  // time of current frame of this animation
  unsigned long m_currentTime = 0;
  // delta time between this frame and previous
  unsigned long m_deltaTime = 0;
  // total run time of this animation
  unsigned long m_runTime = 0;
protected:
  int width;
  int height;
  int depth;
protected:
  // animation helper variables.
  float X, Y, Z;
  // Start animations that use phase at 0
  float phase = 0;
};

class Sinus : public Animation {
private:
  void draw(float);
  void init();
};

class Spiral : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer1, timer2;
  int stage, bottom, top, thickness;
};

class Twinkel : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer1, timer2;
  float seconds;
  int loops;
};

class Rain : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer1, timer2;
  int loops;
};

class Rainbow : public Animation {
private:
  void draw(float);
  void init();
};

class Spin : public Animation {
private:
  void draw(float);
  void init();
private:
  int direction = 1;
  float distort = 0.6*PI;
};

class Sphere : public Animation {
private:
 void draw(float);
 void init();
private:
  int direction;
  float radius;
};

class Bounce : public Animation {
private:
  void draw(float);
  void init();
private:
  Timer timer;
  Object ball;
};

class Starfield : public Animation {
private:
  void draw(float);
  void init();
private:
  static const int numStars = 50;
  bool runOnce = true;
  Vector3 stars[numStars];
};

class Mixer : public Animation {
private:
  void draw(float);
  void init();
public:
  Mixer(Animation* a, Animation* b);
private:
  Animation* a;
  Animation* b;
  Timer timer;
};

class Arrows : public Animation {
private:
  void draw(float);
  void init();
public:
  static constexpr uint16_t bitmap[] = // 9 + 7 + 9 + 7
    {0x10,0x38,0x7C,0xFE,0x38,0x38,0x38,0x38,0x38,
          0x00,0x00,0x00,0x00,0x00,0x00,0x00,
     0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
          0x00,0x00,0x00,0x00,0x00,0x00,0x00};

  static constexpr uint8_t rotation[] = // 8 + 8 + 8 + 8
    {0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7, 8,8,8,8,8,8,8,8, 8,7,6,5,4,3,2,1};

  static const int rows = sizeof(rotation);
private:
  float dx, dy, dz;
  ColorBlender bx, by, bz;
};

class OutsideScroller : public Animation {
public:
  OutsideScroller(const char*);
private:
  void draw(float);
  void init();
private:
  char* text;
  unsigned int numChars;
  uint16_t bitPos;
  uint16_t startPos;
  Timer timer;
  static constexpr uint8_t rotation[] = // 8 + 8 + 8 + 8
    {0,0,0,0,0,0,0,0, 0,1,2,3,4,5,6,7, 8,8,8,8,8,8,8,8, 8,7,6,5,4,3,2,1};
};

class InsideScroller : public Animation {
public:
  InsideScroller(const char*);
private:
  void draw(float);
  void init();
private:
  char* text;
  int numChars;
  int16_t charPos;
  uint16_t zPos;
  Timer timer;
};

class Tree : public Animation {
public:
  static constexpr uint16_t bitmap[] = {
//  0x000,0x000,0x000,0x000,0x010,0x000,0x000,0x000,0x000, // y = 0
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000, // y = 0
    0x038,0x07c,0x0fe,0x1ff,0x1ff,0x1ff,0x0fe,0x07c,0x038, // y = 1
    0x000,0x010,0x038,0x07c,0x0fe,0x07c,0x038,0x010,0x000, // y = 2
    0x000,0x000,0x010,0x038,0x07c,0x038,0x010,0x000,0x000, // y = 3
    0x000,0x010,0x038,0x07c,0x0fe,0x07c,0x038,0x010,0x000, // y = 4
    0x000,0x000,0x010,0x038,0x07c,0x038,0x010,0x000,0x000, // y = 5
    0x000,0x000,0x000,0x010,0x038,0x010,0x000,0x000,0x000, // y = 6
    0x000,0x000,0x000,0x000,0x010,0x000,0x000,0x000,0x000, // y = 7
    0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000  // y = 8
//  0x000,0x000,0x000,0x000,0x010,0x000,0x000,0x000,0x000  // y = 8
  };
private:
  void draw(float);
  void init();
private:
  int numLeafs;
  Timer timer1, timer2;
  Object star;
  Object trunk;
  Object tree[150];
};

class Fireworks : public Animation {
private:
  void draw(float);
  void init();
private:
  Vector3 source, target, delta, velocity;
  int numDebris;
  Object missile;
  Object debris[40];
};

class Voxicles : public Animation {
private:
  void draw(float);
  void init();
private:
  float angle = 0;
  float qAngle = 0;
  float qDivider = 1;
  Timer timer1, timer2;
  float sAngle = 0;
  int direction = 1;
  float step = 0;
};
#endif
