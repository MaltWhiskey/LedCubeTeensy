#include "Cube.h"
#include "Quaternion.h"

// Set dimensions of the Led Cube and initialize the OctadecaTLC5940 driver.
Cube::Cube(int width, int height, int depth){
	m_Width = width;
	m_Height = height;
	m_Depth = depth;
}
/* The map function maps the distance (in) on a scale of inMin to inMax to the
   distance (out) on a scale of outMin to outMax */
float Cube::map(float in, float inMin, float inMax, float outMin, float outMax) {
    return (in-inMin)/(inMax-inMin)*(outMax-outMin) + outMin;
}
// Move the cube down, clear the top layer
void Cube::down() {
  for(int x=0; x < m_Width;x++)
  for(int z=0; z < m_Depth;z++) {
    for(int y=1;y < m_Height;y++)
      setVoxel(x,y-1,z, getDisplayedVoxel(x,y,z));
    setVoxel(x,m_Height-1,z, Color::BLACK);
  }
}
// Copy the displayed cube to the rendering cube
void Cube::copy() {
  for(int x=0;x < m_Width;x++)
  for(int y=0;y < m_Height;y++)
  for(int z=0;z < m_Depth;z++)
    setVoxel(x,y,z, getDisplayedVoxel(x,y,z));
}
// Fades the entire cube to zero in the specified amount of steps (0.99 = 0)
// Fades very fast in the beginning, but slows down quickly. Suffers from integer
// truncating at the end speeding up the process again. Without knowing the original
// value linear interpolation is impossible. This looks very impressive regardless :-)
void Cube::fade(int steps) {
  double multiplier = pow(0.99/4096, 1.0/steps);
  for(int x=0;x < m_Width;x++)
  for(int y=0;y < m_Height;y++)
  for(int z=0;z < m_Depth;z++) {
    Color rgb = getDisplayedVoxel(x,y,z);
    rgb.R *= multiplier;
    rgb.G *= multiplier;
    rgb.B *= multiplier;
    setVoxel(x,y,z, rgb);
  }
}
// Fades the entire cube to zero in the specified amount of seconds
void Cube::fade(float seconds, float dt) {
  if(dt > 0)
	  fade(seconds/dt);
  else
	  copy();
}
void Cube::setVoxel(Vector3& v, Color c) {
  setVoxel(v.x, v.y, v.z, c);
}
void Cube::mergeVoxel(Vector3& v, Color c_) {
  mergeVoxel(v.x,v.y,v.z, c_);
}
void Cube::mergeVoxel(int x, int y, int z, Color c_) {
  Color c = getRenderingVoxel(x,y,z);
  c.R = (c_.R + c.R) >> 1;
  c.G = (c_.G + c.G) >> 1;
  c.B = (c_.B + c.B) >> 1;
  setVoxel(x,y,z,c);
}
void Cube::radiateVoxel(Vector3& vOrigin, Color c_, float distance) {
  for(int x=vOrigin.x-1;x<vOrigin.x+2;x++)
  for(int y=vOrigin.y-1;y<vOrigin.y+2;y++)
  for(int z=vOrigin.z-1;z<vOrigin.z+2;z++) {
	Vector3 vLight = Vector3(x,y,z) ;
	if(vLight.inside(m_Width,m_Height,m_Depth)) {
	  Vector3 vDistance = vOrigin - vLight;
	  float radius = distance*vDistance.magnitude();
	  // inverse square rule super powered by inverse/(radius^5+1)
	  float intensity = 1/(1 + (radius*radius*radius*radius*radius));
	  Color c = Color(c_.R*intensity,c_.G*intensity,c_.B*intensity);
	  mergeVoxel(vLight.x, vLight.y, vLight.z, c);
	}
  }
}
void Cube::animate() {
  // render one animation frame using the cube dimensions
  animation->animate(m_Width, m_Height, m_Depth);
  // when an animation is finished it resets and has status not running
  if(!animation->running()) {
	animation = animations[random(0,sizeof(animations)/sizeof(Animation*))];
  }
  // wait for vertical blank and than switch rendering and displayed buffers
  update();
}
