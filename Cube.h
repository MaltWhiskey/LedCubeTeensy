#ifndef CUBE_H
#define CUBE_H
#include "OctadecaTLC5940.h"
#include "Animation.h"
/*       3D Led cube coordinate system

              + + + + + + + + +
             ++              ++
            + +             + +
           +  +            +  +
    Y     +   +           +   +     Z
    |    +    +          +    +    /
    |   +     +         +     +   /
    |  +      +        +      +  /
    8 + + + + + + + + + + + + + 8
    7 +      +        +      + 7
    6 +     +         +     + 6
    5 +    +          +    + 5
    4 +   +           +   + 4
    3 +  +            +  + 3
    2 + +             + + 2
    1 ++              ++ 1
    0 + + + + + + + + + 0
    0 1 2 3 4 5 6 7 8---X               */
class Cube : public OctadecaTLC5940 {
private:
  int m_Width;
  int m_Height;
  int m_Depth;
private:
  void fade(int steps);
public:
  Cube(int width, int height, int depth);
  using OctadecaTLC5940::setVoxel;
  void setVoxel(Vector3& v, Color c);
  void mergeVoxel(int x, int y, int z, Color);
  void mergeVoxel(Vector3& v, Color c);
  void radiateVoxel(Vector3& v, Color c, float distance);
  float map(float value, float currentMin, float currentMax, float newMin, float newMax);
  void down();
  void copy();
  void fade(float seconds, float dt);
  void animate();
private:
  Sinus sinus = Sinus();
  Spiral spiral = Spiral();
  Twinkel twinkel = Twinkel();
  Rain rain = Rain();
  Rainbow rainbow = Rainbow();
  Spin spin = Spin();
  Starfield starfield = Starfield();
  Sphere sphere = Sphere();
  Arrows arrows = Arrows();
  Bounce bounce = Bounce();
  Tree tree = Tree();
  Voxicles voxicles = Voxicles();

  Fireworks fireworks1 = Fireworks();
  Fireworks fireworks2 = Fireworks();
  InsideScroller insideScroller = InsideScroller("Lucan");
  InsideScroller insideScroller2 = InsideScroller("Calista is jarig");
  OutsideScroller outsideScroller = OutsideScroller("\x1F""Calista 6 Jaar\x1F");
  OutsideScroller outsideScroller2 = OutsideScroller("\x1FMascha\x1F");
  Mixer mixer1 = Mixer(&fireworks1, &fireworks2);
  Mixer mixer2 = Mixer(&insideScroller2, &mixer1);
private:
  Animation* animation = &mixer2;
  Animation* animations[17] = {
    &sinus, &spiral, &twinkel, &rain, &rainbow, &spin,
    &starfield, &sphere, &arrows, &bounce, &tree, &voxicles,
	&mixer1, &mixer2, &insideScroller2, &outsideScroller, &outsideScroller2
  };
};
#endif
