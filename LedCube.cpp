#include "Cube.h"
#include "Color.h"
#include "Util.h"
/*---------------------------------------------------------------------------------------
 * Globals
 *-------------------------------------------------------------------------------------*/
Cube cube(9,9,9);
ColorWheel colorwheel(150);
NoiseGenerator generator;
/*---------------------------------------------------------------------------------------
 * Initialize setup parameters
 *-------------------------------------------------------------------------------------*/
void setup() {
  Serial.begin(9600);
  delay(2000);
  colorwheel.add(Color::RED);
  colorwheel.add(Color::GREEN);
  colorwheel.add(Color::BLUE);
  colorwheel.add(Color::RED);
  colorwheel.add(Color::GREEN);
  colorwheel.add(Color::BLUE);
  colorwheel.add(Color::BLACK);
}
/*---------------------------------------------------------------------------------------
 * Start the main loop
 *-------------------------------------------------------------------------------------*/
void loop() {
  cube.animate();
}
