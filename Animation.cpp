#include "Animation.h"
#include "Cube.h"
#include "Color.h"
#include "OctadecaTLC5940.h"
#include "Font.h"
#include "Util.h"

extern Cube cube;
extern ColorWheel colorwheel;
extern NoiseGenerator generator;
/*---------------------------------------------------------------------------------------
 * ANIMATION INTERFACE
 *-------------------------------------------------------------------------------------*/
Animation::~Animation() { }
void Animation::animate(int width_, int height_, int depth_) {
  width = width_;
  height = height_;
  depth = depth_;

  m_currentTime = micros();
  if(m_startTime==0) {
	m_startTime = m_currentTime;
	m_lastTime = m_currentTime;
	init();
  }
  m_deltaTime = m_currentTime-m_lastTime;
  m_runTime = m_currentTime-m_startTime;
  draw(m_deltaTime/1000000.0f);
  m_lastTime = m_currentTime;
}
void Animation::restart() {
  m_startTime=0;
}
bool Animation::running() {
  return m_startTime!=0;
}
/*---------------------------------------------------------------------------------------
 * SINUS
 *-------------------------------------------------------------------------------------*/
void Sinus::init() { }
void Sinus::draw(float dt) {
  phase += PI*dt;
  colorwheel.turn(-dt/10.0f);

  for(int x=0;x < width;x++) {
    X = cube.map(x, 0, width-1, -2, 2);
    for(int z=0;z < depth;z++) {
      Z = cube.map(z, 0, depth-1,-2,2);
      Y = sinf(phase + sqrtf(X*X + Z*Z));
      Y = round(cube.map(Y, -1, 1, 0, height-1));
      cube.setVoxel(x,(int)Y,z, colorwheel.color(Y*0.01f));
    }
  }

  if(phase/(4*PI) >= 1) {
	phase-=4*PI;
	restart();
  }
}
/*---------------------------------------------------------------------------------------
 * SPIRAL
 *-------------------------------------------------------------------------------------*/
void Spiral::init() {
  phase = 0;
  bottom = 0;
  top = 0;
  thickness = 1;
  stage = 0;
  timer1 = 0.2f;
  timer2 = 2.0f;
}

void Spiral::draw(float dt){
  phase += 2*PI*dt;
  colorwheel.turn(dt/10.0f);

  for(int y=bottom;y<top;y++)
  for(int i=0;i<thickness;i++) {
    X = sinf(phase + cube.map((float)y, 0, height-1, 0, 2*PI) + i*PI/60);
    Z = cosf(phase + cube.map((float)y, 0, height-1, 0, 2*PI) + i*PI/60);
    X = cube.map((float)X, -1.1f, 0.9f, 0, width-1);
    Z = cube.map((float)Z, -1.1f, 0.9f, 0, height-1);
    cube.setVoxel(X,y,Z,colorwheel.color(y*0.01f));
  }
  if(timer1.ticks()) {
    int state = 0;
    if(stage == state++)
      top < height ? top++ : stage++;
    if(stage == state++)
      thickness < 25 ? thickness++ : stage++;
    if(stage == state++)
      if(timer2.ticks()) stage++;
    if(stage == state++)
      bottom < height ? bottom++ : stage++;
    if(stage == state++)
      restart();
  }
}
/*---------------------------------------------------------------------------------------
 * TWINKLE
 *-------------------------------------------------------------------------------------*/
void Twinkel::init() {
  timer1 = generator.nextRandom(0.01f, 0.05f);
  timer2 = generator.nextRandom(1.00f, 4.00f);
  seconds= generator.nextRandom(0.50f, 4.00f);
  if(loops==0) loops = random(4,10);
}
void Twinkel::draw(float dt) {
  cube.fade(seconds, dt);
  if(timer1.ticks()) {
    Color color;
    color.random();
	cube.setVoxel(random(width),random(height),random(depth), color);
  }
  if(timer2.ticks()) {
	if(--loops==0) restart();
	else init();
  }
}
/*---------------------------------------------------------------------------------------
 * RAIN
 *-------------------------------------------------------------------------------------*/
void Rain::init() {
  timer1 = generator.nextRandom(0.025f, 0.050f);
  timer2 = generator.nextRandom(1.000f, 4.000f);
  if(loops==0) loops = random(3,8);
}
void Rain::draw(float dt) {
  if(timer1.ticks()) {
	cube.down();
    for(int d=random(0,3);d>0;d--) {
	  Color color;
      int x=random(width);
      int z=random(height);
      color.R=random(0x600);
      color.G=random(0x600);
      color.B=random(0xB00, 0x1000);
      cube.setVoxel(x,height-1,z,color);
    }
  }
  else {
    cube.copy();
  }
  if(timer2.ticks()) {
	if(--loops==0) restart();
	else init();
  }
}
/*---------------------------------------------------------------------------------------
 * RAINBOW
 *-------------------------------------------------------------------------------------*/
void Rainbow::init() {}
void Rainbow::draw(float dt) {
  phase += PI/5*dt;
  colorwheel.turn(sinf(phase)/3*dt);

  for(int x=0;x<width;x++)
  for(int y=0;y<height;y++)
  for(int z=0;z<depth;z++)
    cube.setVoxel(x,y,z, colorwheel.color(0.02f*x+0.01f*y+0.005f*z));

  if(phase/(2*PI) >= 1) {
	phase-=2*PI;
	restart();
  }
}
/*---------------------------------------------------------------------------------------
 * SPIN
 *-------------------------------------------------------------------------------------*/
void Spin::init() { }
void Spin::draw(float dt) {
  phase += 1.25*PI*dt;
  colorwheel.turn(dt/2);

  for(int x=0;x<width;x++)
  for(int y=0;y<height;y++)
  for(int z=0;z<depth;z++) {
    X = (x-((width-1)/2))*sinf(phase+cube.map((float)y,0,height,0,distort));
    Z = (z-((height-1)/2))*cosf(phase+cube.map((float)y,0,height,0,distort));
    if(abs(X-Z) < .8) cube.setVoxel(x,y,z, colorwheel.color(0.02f*y));
  }

  distort+=0.01*PI*direction;
  if(distort > 0.75*PI || distort < 0.25*PI)
    direction*=-1;
  if((phase/(10*PI)) >= 1) {
	phase -= 10*PI;
	restart();
  }
}
/*---------------------------------------------------------------------------------------
 * SPHERE
 *-------------------------------------------------------------------------------------*/
void Sphere::init() {
  radius = 0;
  direction = 1;
}
void Sphere::draw(float dt) {
  radius+=direction*5*dt;
  colorwheel.turn(dt/2);

  for(int s=0;s<=1;s++) {
    for(int x=0;x<width;x++)
    for(int y=0;y<height;y++)
    for(int z=0;z<depth;z++) {
      X = cube.map((float)x,0,width-1,-1,1);
      Y = cube.map((float)y,0,height-1,-1,1);
      Z = sqrtf(-X*X-Y*Y+(radius-s*2.5));
      Z = cube.map(Z,-1,1,0,depth-1);
      if(abs(Z-z) <= 0.8) {
        cube.setVoxel(x,y,z, colorwheel.color(0.015f*y+0.020f*z));
        cube.setVoxel(x,y,depth-1-z, colorwheel.color(0.015f*y+0.020f*z));
      }
    }
  }

  if(radius >= 7.0f)
    direction = -1;
  if(radius < 0)
	restart();
}
/*---------------------------------------------------------------------------------------
 * STARFIELD
 *-------------------------------------------------------------------------------------*/
void Starfield::init() {
  if(runOnce) {
    for(int i=0;i<numStars;i++) {
      stars[i].x = random(0, width);;
      stars[i].y = random(0, height);
	  stars[i].z = random(0, depth);
    }
    runOnce = false;
  }
  phase = PI;
}
void Starfield::draw(float dt) {
  phase+=PI/10*dt;
  colorwheel.turn(dt/10);

  for(int i=0;i<numStars;i++) {
    float sx = 4-abs(stars[i].x - 4);
    float sy = 4-abs(stars[i].y - 4);
    float s = pow(sx*sx + sy*sy, 0.3);
    if (s<1) s = 1;
    stars[i].z += sinf(phase)*dt*s*20;
    if(stars[i].z >= depth) {
      stars[i].x = random(0, width);
      stars[i].y = random(0, height);
      stars[i].z = 0;
    } else if(stars[i].z <= 0) {
      stars[i].x = random(0,width);
      stars[i].y = random(0,height);
      stars[i].z = depth-1;
    }
    cube.setVoxel(stars[i].x,stars[i].y,stars[i].z, colorwheel.color(0));
  }

  if((phase/(6*PI))>=1){
	restart();
  }
  else if((phase/(5*PI))>=1) {
  }
  else if((phase/(4*PI))>=1) {
	phase+=PI;
  }
}
/*---------------------------------------------------------------------------------------
 * BOUNCE
 *-------------------------------------------------------------------------------------*/
void Bounce::init() {
  ball.position = Vector3(width/2.0f, height/2.0f, depth/2.0f);
  ball.velocity = Vector3(generator.nextRandom(5.0f,15.0f),
	generator.nextRandom(5.0f,15.0f), generator.nextRandom(5.0f,15.0f));
  timer = 5.0f;
}
void Bounce::draw(float dt) {
  colorwheel.turn(dt/20);

  for(int x=0;x<width;x++)
  for(int y=0;y<height;y++)
  for(int z=0;z<depth;z++) {
	if(ball.hit(x,y,z, 1.8f))
      cube.setVoxel(x,y,z, colorwheel.color(0));
  }
  ball.bounce(dt, generator.nextRandom(5.0f,20.0f), width, height, depth);

  if(timer.ticks()) restart();
}
/*---------------------------------------------------------------------------------------
 * MIXER
 *-------------------------------------------------------------------------------------*/
Mixer::Mixer(Animation* a_, Animation* b_) {
  a = a_; b = b_;
}
void Mixer::init() {
  timer = 10.0f;
}
void Mixer::draw(float dt) {
  if(!timer.expired()) {
	a->animate(width, height, depth);
    b->animate(width, height, depth);
  }
  else if(!a->running() && !b->running()) {
    restart();
  }
  else {
	if(a->running()) a->animate(width, height, depth);
	if(b->running()) b->animate(width, height, depth);
  }
}
/*---------------------------------------------------------------------------------------
 * ARROWS
 *-------------------------------------------------------------------------------------*/
constexpr uint16_t Arrows::bitmap[32];
constexpr uint8_t Arrows::rotation[32];
void Arrows::init() {
  X=0.0f; Y=0.0f; Z=0.0f;
  dx = 1; dy = 0; dz = 0;
  bx = ColorBlender(2.0f);
  by = ColorBlender(2.0f);
  bz = ColorBlender(2.0f);
}

void Arrows::draw(float dt) {
  X+=10*dt; Y+=15*dt; Z+=20*dt;
  colorwheel.turn(dt/20);
  bx.blend(dt*dx);
  by.blend(dt*dy);
  bz.blend(dt*dz);

  int c, d;
  // loop total number of outside rows 9+7+9+7=32
  for(int r=0;r<rows;r++) {
    c = rotation[r];
    d = rotation[(r+8)%rows];
    // get 9 bit planes and 9 bits in bitmap
    for(int b=0;b<width;b++) {
      if(bitmap[((int)X+r)%rows]&(1<<b)) {
		cube.mergeVoxel(b,c,d, bx.color(Color::BLACK,colorwheel.color(0.00f+X*0.01f)));
	  }
      if(bitmap[((int)Y+r)%rows]&(1<<b)) {
        cube.mergeVoxel(c,b,d, by.color(Color::BLACK,colorwheel.color(0.33f+Y*0.01f)));
      }
      if(bitmap[((int)Z+r)%rows]&(1<<b)) {
        cube.mergeVoxel(d,c,b, bz.color(Color::BLACK,colorwheel.color(0.66f+Z*0.01f)));
      }
    }
  }

  if(dx==1) {
    if(bx.finished()) dy = 1;
    if(by.finished()) dz = 1;
    if(bz.finished()) { dx = -1, dy = 0; dz = 0; }
  }
  else {
    if(bx.finished()) dy = -1;
    if(by.finished()) dz = -1;
    if(bz.finished()) restart();
  }
}
/*---------------------------------------------------------------------------------------
 * SCROLLER
 *-------------------------------------------------------------------------------------*/
constexpr uint8_t OutsideScroller::rotation[32];
OutsideScroller::OutsideScroller(const char* text_) {
  text = (char*)text_;
  numChars	= strlen(text);
  init();
}
void OutsideScroller::init() {
  bitPos = 0;
  startPos = 32;
  timer = 0.08f;
}
void OutsideScroller::draw(float dt) {
  colorwheel.turn(dt/20);

  for(int b=startPos;b<32;b++) {
    int chrOffset = (b+bitPos-startPos)/8;
    int bitOffset = (b+bitPos-startPos)%8;
    if(chrOffset >= numChars) continue;
    uint8_t chr = text[chrOffset];

    for(int y=7;y>=0;y--) {
      uint8_t bitset = charset[chr][7-y];
      if(bitset & (1<<bitOffset)) {
        int x = rotation[(b)%32];
        int z = rotation[(24+b)%32];
        if(chr==0x1F)
          cube.setVoxel(x,y,z,Color::RED);
        else
          cube.setVoxel(x,y,z,colorwheel.color(z*0.01f));
      }
    }
  }

  if(timer.ticks()) {
     startPos > 0 ? startPos-- : bitPos++;
  }
  if(bitPos/8 >= numChars) {
    restart();
  }
}
/*---------------------------------------------------------------------------------------
 * SCROLLER
 *-------------------------------------------------------------------------------------*/
InsideScroller::InsideScroller(const char* text_) {
  text = (char*)text_;
  numChars = strlen(text);
  init();
}
void InsideScroller::init() {
  charPos = 0;
  zPos = 8;
  timer = 0.06f;
}
void InsideScroller::draw(float dt) {
  colorwheel.turn(dt/20);

  for(int x=1;x<=8;x++) {
    uint8_t chr = text[charPos%numChars];
    for(int y=7;y>=0;y--) {
      uint8_t bitset = charset[chr][7-y];
      if(bitset & (1<<(x-1))) {
        cube.setVoxel(x,y,zPos,colorwheel.color(zPos*0.02f));
      }
    }
  }
  if(timer.ticks())
     zPos > 0 ? zPos-- : (zPos = 8, charPos++);
  if(charPos >= numChars) {
	restart();
  }
}
/*---------------------------------------------------------------------------------------
 * TREE
 *-------------------------------------------------------------------------------------*/
constexpr uint16_t Tree::bitmap[];
void Tree::init() {
  numLeafs = 0;
  star  = { Vector3(4,8,4) };
  star.cb = ColorBlender(Color::BLACK, Color::WHITE, 1.0f);
  trunk = { Vector3(4,0,4) };

  for(int y=0;y<height;y++)
  for(int x=0;x<width;x++)
  for(int z=0;z<depth;z++)
  if(bitmap[y*9+z]&(1<<x)) {
	tree[numLeafs] = { Vector3(x,y,z) };
	tree[numLeafs].cb = ColorBlender(Color::GREEN, Color::GREEN, 1.0f);
    numLeafs++;
  }
  timer1 = 0.025f;
  timer2 = 2.0f;
}
void Tree::draw(float dt) {
  colorwheel.turn(dt);

  if(timer1.ticks()) {
    int i = random(0,numLeafs);
      tree[i].cb = ColorBlender(Color::GREEN, colorwheel.color(0),
        generator.nextRandom(0.20f, 1.0f));
  }
  Color c = star.cb.pulse(dt);
  cube.setVoxel(star.position, c);
  cube.setVoxel(trunk.position, Color::BROWN);

  for(int i=0;i<numLeafs;i++) {
	c = tree[i].cb.pulse(dt);
	if(c == Color::GREEN) tree[i].cb.target = Color::GREEN;
    cube.setVoxel(tree[i].position, c);
  }
  if(timer2.ticks()) restart();
}
/*---------------------------------------------------------------------------------------
 * VOXICLES
 *-------------------------------------------------------------------------------------*/
void Voxicles::init() {
  timer1 = 30.0f;
  qAngle/=qDivider;
  qDivider = random(1,8);
  qAngle*=qDivider;
  sAngle=0;
  step=0;
  direction=1;
}
void Voxicles::draw(float dt) {
  angle  += (150.0f * dt);
  qAngle += (200.0f * dt);
  colorwheel.turn(dt/20);

  if((direction == 1) & (step < 50))
	step++;
  else {
    sAngle += direction * 30.0f * dt;
    if(sAngle >=90)
	  sAngle=90;
    if(sAngle < 0)
	  sAngle=0;
  }

  int ci = 0;            // color index
  const float cd = 0.03; // color delta
  struct Orbit {
    float x,y,z;
    Color c;
  };
  Orbit orbit[6] = {
    4,0,0,colorwheel.color(ci++*cd),0,0,-4,colorwheel.color(ci++*cd),
	0,4,0,colorwheel.color(ci++*cd),0,-4,0,colorwheel.color(ci++*cd),
	0,0,4,colorwheel.color(ci++*cd),-4,0,0,colorwheel.color(ci++*cd)};

  Vector3 rotations[6];
  // change the axis angles all at a different rate
  rotations[0] = Vector3(+sinf(angle/90),+sinf(angle/70),-sinf(angle/90));
  rotations[1] = Vector3(+sinf(angle/80),-sinf(angle/70),-sinf(angle/99));
  rotations[2] = Vector3(-sinf(angle/99),+sinf(angle/90),-sinf(angle/80));
  rotations[3] = Vector3(-sinf(angle/90),-sinf(angle/90),+sinf(angle/99));
  rotations[4] = Vector3(-sinf(angle/70),-sinf(angle/80),-sinf(angle/90));
  rotations[5] = Vector3(-sinf(angle/99),+sinf(angle/70),+sinf(angle/80));

  for(int i=0;i<6;i++) {
    Vector3 vAxis = rotations[i];
    // rotate quaternion around the axis angles (angle represent the speed 180=max)
    Quaternion qr = Quaternion(qAngle/qDivider, vAxis);
    qr.convertAxisAngle();

    Vector3 vr = Vector3(orbit[i].x,orbit[i].y,orbit[i].z);
    vr*=sinf(0.01f+2*PI*sAngle/360);
	qr.rotate(vr);
	vr+=Vector3(4,4,4);
	Color c = Color(Color::BLACK, orbit[i].c,step,50);
    cube.radiateVoxel(vr, c, 1.3f);
  }

  if(timer1.ticks())
    direction = -1;
  if(direction == -1 and sAngle == 0) {
    if(--step==0) restart();
  }
}
