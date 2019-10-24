#include "Color.h"

const Color Color::BLACK    (0x000, 0x000, 0x000);
const Color Color::WHITE    (0xFFF, 0xFFF, 0xFFF);
const Color Color::RED      (0xFFF, 0x000, 0x000);
const Color Color::GREEN    (0x000, 0xFFF, 0x000);
const Color Color::BLUE     (0x000, 0x000, 0xFFF);
const Color Color::YELLOW   (0xFFF, 0xFFF, 0x000);
const Color Color::ORANGE   (0xFFF, 0x800, 0x000);
const Color Color::PINK     (0x800, 0x300, 0x800);
const Color Color::CYAN     (0x000, 0xFFF, 0xFFF);
const Color Color::MAGENTA  (0xFFF, 0x000, 0xFFF);
const Color Color::BROWN    (0x7B0, 0x400, 0x130);

Color::Color(){
  R=0x000;
  G=0x000;
  B=0x000;
}

Color::Color(uint16_t R_, uint16_t G_, uint16_t B_){
  R=R_;
  G=G_;
  B=B_;
}

/* Creates a Color between source and target scaled by the number of steps
 * if step == 0 gives the source color
 * if step == steps gives the target color
 * if step > 0 and < steps gives a color in between */
Color::Color(Color source, Color target, uint16_t step, uint16_t steps) {
  R = source.R + (step*((long)target.R - (long)source.R))/steps;
  G = source.G + (step*((long)target.G - (long)source.G))/steps;
  B = source.B + (step*((long)target.B - (long)source.B))/steps;
}

boolean Color::operator==(Color c) {
  return ((c.R==R) & (c.G==G) & (c.B==B));
}

void Color::print() {
  Serial.print(R); Serial.print(",");
  Serial.print(G); Serial.print(",");
  Serial.print(B); Serial.println();
}

void Color::random() {
  R=::random(4096);
  G=::random(4096);
  B=::random(4096);
}

bool Color::isBlack() {
  return (R|G|B)==0;
}

ColorWheel::ColorWheel(int steps){
  m_steps = steps;
}

void ColorWheel::add(Color color) {
  m_colors.push_back(color);
  create();
  m_size = (int)m_colorWheel.size();
}

void ColorWheel::create() {
  m_colorWheel.clear();
  for(int c=0;c<(int)m_colors.size();c++)
    for(int i=0;i<m_steps;i++)
      m_colorWheel.push_back(Color(m_colors[c],m_colors[(c+1)%(int)m_colors.size()],i,m_steps));
}

/* Get the color relative from the offset and the wheel position */
Color ColorWheel::color(float percentage) {
  percentage+=m_wheelPosition;
  percentage-=(int)percentage;
  if(percentage<0.0f) percentage += 1;
  // Better safe than sorry check. Floating points are a bitch...
  if(percentage>=1.0f) percentage = 0;
  percentage=percentage*(m_size);
  return m_colorWheel[percentage];
}

/* Turns the color wheel amount positions forward or backwards */
void ColorWheel::turn(float percentage) {
  percentage+=m_wheelPosition;
  percentage-=(int)percentage;
  if(percentage<0.0f) percentage += 1;
  if(percentage>=1.0f) percentage = 0;
  m_wheelPosition=percentage;
}
ColorBlender::ColorBlender() {}
ColorBlender::ColorBlender(float targetTime_) {
	elapsedTime = 0;
	targetTime = targetTime_;
}
ColorBlender::ColorBlender(Color source_, Color target_, float targetTime_) {
	source = source_;
	target = target_;
	elapsedTime = 0;
	targetTime = targetTime_;
}
// Can blend in both directions +dt or -dt
Color ColorBlender::blend(float dt) {
  if (dt == 0) direction = 0;
  else if(dt > 0) direction = +1;
  else if(dt < 0) direction = -1;
  elapsedTime+=dt;
  if(elapsedTime >= targetTime) {
    elapsedTime = targetTime;
    // return the target because rounding errors could be a bit off
    return target;
  }
  if(elapsedTime < 0) {
	elapsedTime = 0;
	// return source
	return source;
  }
  return Color(source, target, 255 * elapsedTime / targetTime, 255);
}
Color ColorBlender::color(Color source_, Color target_) {
  source = source_;
  target = target_;
  return Color(source, target, 255 * elapsedTime / targetTime, 255);
}
// pulse swaps source and target.
Color ColorBlender::pulse(float dt) {
  elapsedTime+=dt;
  if(elapsedTime >= targetTime) {
    elapsedTime = 0;
    Color c = target;
    target = source;
    source = c;
    // no rounding errors here because 0/targetTime is still zero
  }
  return Color(source, target, 255 * elapsedTime / targetTime, 255);
}
bool ColorBlender::finished() {
  if(direction == +1)
    return (elapsedTime == targetTime);
  else if(direction == -1)
	return (elapsedTime == 0);
  return false;
}
