#include "Animation.h"
#include "Cube.h"
#include "Color.h"
#include "Util.h"

extern Cube cube;
extern ColorWheel colorwheel;
extern NoiseGenerator generator;
/*---------------------------------------------------------------------------------------
 * FIREWORKS
 *-------------------------------------------------------------------------------------*/
void Fireworks::init() {
  // calculate source normally divided from source area
  source = Vector3(
    generator.nextGaussian((float)width/2.0f, 1.5f), 0,
    generator.nextGaussian((float)depth/2.0f, 1.5f));
  // calculate targets normally divided from target area
  target = Vector3(
    generator.nextGaussian((float)width/2.0f, 1.5f),
    generator.nextGaussian((float)2.0f*height/3.0f, 1.5f),
    generator.nextGaussian((float)depth/2.0f, 1.5f));
  // calculate the deltas
  delta = target - source;
  // Assign a time in seconds to reach the target
  float t = generator.nextGaussian(0.45f, 0.15f, 2.5f);
  // Determine directional velocities in pixels per second
  velocity = delta/t;
  // Set missile source, velocity and gravity
  missile.position = source;
  missile.velocity = velocity;
  missile.gravity = Vector3(0,-10.0f,0);
}

void Fireworks::draw(float dt) {
  if(target.y > 0) {
	Vector3 temp = missile.position;
    missile.move(dt);
    if ((temp.y > missile.position.y) | (missile.position.y > target.y))  {
      target.y = 0;
      // If target is reached the missile is exploded and debris is formed
      numDebris = random(20,40);
      // Overall exploding power of arrow for all debris
      float pwr = generator.nextRandom(5.0f,12.0f);
      float colorturn = generator.nextRandom(0.0f,1.0f);
      colorwheel.turn(colorturn);
      for(int i=0;i<numDebris;i++) {
        // Debris is formed at target and has random velocities depending on overall power
    	Vector3 explode = Vector3(generator.nextRandom(-pwr,pwr),
    	  generator.nextRandom(-pwr,pwr), generator.nextRandom(-pwr,pwr));
        debris[i] = { temp, explode, Vector3(0,-10.0f,0) };
    	debris[i].cb = ColorBlender(colorwheel.color(0.005f*i), Color::BLACK,
    	  generator.nextRandom(1,2));
        }
        colorwheel.turn(-colorturn);
      }
    else if(missile.inside(width,height,depth))
	  cube.setVoxel(missile.position, Color::WHITE);
  }
  if(target.y==0) {
    int visible = 0;
    for(int i=0;i<numDebris;i++) {
	  debris[i].move(dt);
	  debris[i].drag(dt, 0.05f);
      Color color = debris[i].cb.blend(dt);
      if(!color.isBlack())
    		visible++;
      if(debris[i].position.y < 0)
        debris[i].position.y = 0;
      if(debris[i].inside(width,height,depth))
    		cube.setVoxel(debris[i].position, color);
    }
    if(visible==0) {
      restart();
    }
  }
}
