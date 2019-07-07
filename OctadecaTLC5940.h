#ifndef OCTADECATLC5940_H
#define OCTADECATLC5940_H
/*
    Pin configuration of the TLC5940, 28-Pin PDIP top view

          ----O----
    OUT1 |1      28| OUT0
    OUT2 |2      27| VPRG  -> GND
    OUT3 |3      26| SIN   <-
    OUT4 |4   T  25| SCLK  <-
      .  |5   L  24| XLAT  <-
      .  |6   C  23| BLANK -> 10K Resistor  -> VCC
      .  |7   5  22| GND
      .  |8   9  21| VCC   -> 104 Capacitor -> GND
      .  |9   4  20| IREF  ->  2K Resistor  -> GND
      .  |10  0  19| DCPRG -> VCC
      .  |11     18| GSCLK <-
      .  |12     17| SOUT  -> SIN next TLC
      .  |13     16| XERR
    OUT14|14     15| OUT15
          ---------

    OUT15 MSB  ....  OUT0 LSB
 */
#include <SPI.h>
#include <DMAChannel.h>
#include <kinetis.h>
#include <core_pins.h>
#include <stdint.h>
#include <math.h>
#include "Color.h"

// Definition of the hardware for using 18xTLC5940
#define CHANNELS    16 * 18
#define CHNBITS     CHANNELS * 12
#define CHNBYTES    CHNBITS / 8
#define SPISPEED    5000000

// Definition of the hardware layers
#define X_LAYERS	9
#define Y_LAYERS	9
#define Z_LAYERS	9

/* These are the output pins inputing to the TLC5940. XLAT, BLANK and GSCLK are generated
 * by hardware timers. If you change these pins you also need to change the timer setup
 * and pin configuration, look for the pin alternate functions in the K64 manual, K64
 * signal multiplexing and pin Assignments chapter 10.3.1. */
#define XLAT    3
#define BLANK   4
#define GSCLK   5

/* Running 18 TLC'S sending data through bit banging will take a lot of time, so I'll
 * use SPI over DMA instead. The SPI library uses SCK on pin 13, MOSI on pin 11 and MISO
 * on pin 12. We only need the data and clock pin. The pin's can be changed, see library
 * documentation on how to do that. */
#define SIN     11
#define SCLK    13

/* CORE_PIN3_CONFIG configures Pin Control Register PTA12.
 * Pin Mux Control, MUX (bit 10-8) -> Alternative 1 PTA12
 * Pin Mux Control, MUX (bit 10-8) -> Alternative 3 FTM1_CH0
 * Drive Strength Enable, DSE (bit 6) High Drive Strength
 * Slew Rate Enable, SRE (bit 2) Slow slew rate */
#define XLAT_PULSE      CORE_PIN3_CONFIG = PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE
#define XLAT_NOPULSE    CORE_PIN3_CONFIG = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE

/* CORE_PIN4_CONFIG configures Pin Control Register PTA13.
 * Pin Mux Control, MUX (bit 10-8) -> Alternative 1 PTA13
 * Pin Mux Control, MUX (bit 10-8) -> Alternative 3 FTM1_CH1
 * Drive Strength Enable, DSE (bit 6) High Drive Strength
 * Slew Rate Enable, SRE (bit 2) Slow slew rate */
#define BLANK_PULSE     CORE_PIN4_CONFIG = PORT_PCR_MUX(3) | PORT_PCR_DSE | PORT_PCR_SRE
#define BLANK_NOPULSE   CORE_PIN4_CONFIG = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE

/* CORE_PIN5_CONFIG configures Pin Control Register PTD7.
 * Pin Mux Control, MUX (bit 10-8) -> Alternative = 1 PTD7
 * Pin Mux Control, MUX (bit 10-8) -> Alternative = 2 CMT_IRO
 * Drive Strength Enable, DSE (bit 6) High Drive Strength
 * Slew Rate Enable, SRE (bit 2) Slow slew rate */
#define GSCLK_PULSE     CORE_PIN5_CONFIG = PORT_PCR_MUX(2) | PORT_PCR_DSE | PORT_PCR_SRE
#define GSCLK_NOPULSE   CORE_PIN5_CONFIG = PORT_PCR_MUX(1) | PORT_PCR_DSE | PORT_PCR_SRE

/* This is the timing specific configuration. This is bus speed specific, so look at your
 * scope to get both timers in synch. The first pulse of GSCLK, BLANK and XLAT should all
 * be overlapping where BLANK encapsulates XLAT and XLAT encapsulates GSCLK. Here after
 * are GSCNT amount of GSCLK pulses and than the same encapsulation should happen to
 * restart the GSCYCLE of the TLC5940. */

// This defines the HIGH and LOW time of 1 GSCLK pulse, keep CGH1+CGL1 even numbered
// as to not run into errors in calculating the period.
#define CGH1    6
#define CGL1    10
#define FTMDIV  4
// Time of one GSCLK period
#define PERIOD  (CGH1+CGL1)/2
// Amount of GSCLK pulses for a complete cycle
#define GSCNT   4096
/* FTMOD takes the CGH1 and CGL1 and amount of GSCNT needed to calculate
 * the MODULO of the FTM timer, divide by prescaler. */
#define FTMOD   (PERIOD*GSCNT)/FTMDIV
/* Match value where XLAT will get pulsed (center aligned) */
#define C0V     FTMOD-(4/FTMDIV)-3
/* Match value where BLANK will get pulsed (center aligned) */
#define C1V     FTMOD-(4/FTMDIV)-2
/* When the CNT passes from negative to 0 the interrupt is fired without */
#define CNTIN  FTMOD
/* Refresh rate in Hertz of the entire cube. Layers run at about 915Hz divide this by
 * Y-layers to get the refresh rate of the cube. The refresh rate is NOT used.
 * Animations use the elapsed time to adjust animation speed accordingly */
#define REFRESH_RATE  (F_BUS/(GSCNT*(CGH1+CGL1))/Y_LAYERS)

class OctadecaTLC5940 {
private:
  /* The memory of the entire cube, double buffered */
  Color m_rgbCube[2][X_LAYERS][Y_LAYERS][Z_LAYERS];
  /* One buffer is currently being used for display and the other buffer is the canvas
   * for rendering. These buffers will swap places after a call to update, this is when
   * a new frame is ready to be displayed and right before the bottom layer is about to
   * be turned on (top to bottom vertical blank). While the top layer is turned on the
   * data for the bottom layer (y=0) is being send in. */
  volatile int m_displayedCube = 0;
  volatile int m_renderingCube = 1;
  /* When the animation routines have a frame ready update is called and a buffer switch
   * will be done right before the bottom layer data is being send in */
  volatile bool m_nextFrameReady = false;
  /* Hardware LED address mapping for getting the right LED offset see PCB schematic.
   * The table has the following layout: LED0B, LED0G, LED0R, LED1B, ..., LED81G, LED81R
   * There are some holes in this table, some are spare led addresses, others are unused.
   * Every led has a common anode and 3 cathodes and every cathode has a separate address
   *
   * There are [9] rows of LEDs with [9] LEDs in each row and every led has [3] colors.*/
  uint16_t m_ledChannel[X_LAYERS][Z_LAYERS][3] = {
  {{  3,   4,   5},{  6,   7,   8},{  9,  10,  11},
   { 99, 100, 101},{102, 103, 104},{105, 106, 107},
   {195, 196, 197},{198, 199, 200},{201, 202, 203}},
  {{  1,   2,   0},{ 22,  23,  24},{ 25,  26,  27},
   { 97,  98,  96},{118, 119, 120},{121, 122, 123},
   {193, 194, 192},{214, 215, 216},{217, 218, 219}},
  {{ 17,  18,  16},{ 19,  20,  21},{ 28,  29,  30},
   {113, 114, 112},{115, 116, 117},{124, 125, 126},
   {209, 210, 208},{211, 212, 213},{220, 221, 222}},
  {{ 35,  36,  37},{ 38,  39,  40},{ 41,  42,  43},
   {131, 132, 133},{134, 135, 136},{137, 138, 139},
   {227, 228, 229},{230, 231, 232},{233, 234, 235}},
  {{ 33,  34,  32},{ 54,  55,  56},{ 57,  58,  59},
   {129, 130, 128},{150, 151, 152},{153, 154, 155},
   {225, 226, 224},{246, 247, 248},{249, 250, 251}},
  {{ 49,  50,  48},{ 51,  52,  53},{ 60,  61,  62},
   {145, 146, 144},{147, 148, 149},{156, 157, 158},
   {241, 242, 240},{243, 244, 245},{252, 253, 254}},
  {{ 67,  68,  69},{ 70,  71,  72},{ 73,  74,  75},
   {163, 164, 165},{166, 167, 168},{169, 170, 171},
   {259, 260, 261},{262, 263, 264},{265, 266, 267}},
  {{ 65,  66,  64},{ 86,  87,  88},{ 89,  90,  91},
   {161, 162, 160},{182, 183, 184},{185, 186, 187},
   {257, 258, 256},{278, 279, 280},{281, 282, 283}},
  {{ 81,  82,  80},{ 83,  84,  85},{ 92,  93,  94},
   {177, 178, 176},{179, 180, 181},{188, 189, 190},
   {273, 274, 272},{275, 276, 277},{284, 285, 286}}};
  /* This is the pin mapping for multiplexing the layers of the led cube, starting with
   * the bottom layer. All layers are switched with a mosfet LOW=ON, HIGH=OFF, they are
   * connected with a 1K pull up resistor as to not switch them on at boot up time. */
  uint8_t m_layerPin[Y_LAYERS + 1] = {14, 15, 16, 17, 18, 19, 20, 21, 22, 14};
  /* Initialize settings for transferring data using DMA and SPI */
  DMAChannel m_dmaChannel;
  SPISettings m_spiSettings = SPISettings(SPISPEED, MSBFIRST, SPI_MODE0);
  /* Number of bytes that are needed to send all bits to the TLC's. (12 bits/channel) */
  uint8_t m_channelBuffer[CHNBYTES];
  /* There should be only one displayed layer that is set to LOW, all other layers should
   * be set to HIGH. The displaying of the layers will start at the bottom (y=0). Every
   * cycle turns off the current layer first and than turns on the next one. */
  volatile uint8_t m_LayerOffset = 0;
  volatile uint8_t m_currentLayer = m_layerPin[m_LayerOffset];
  volatile uint8_t m_nextLayer = m_layerPin[m_LayerOffset+1];
public:
  OctadecaTLC5940();
  void setVoxel(int x, int y, int z, Color c);
  Color getRenderingVoxel(int x, int y, int z);
  Color getDisplayedVoxel(int x, int y, int z);
  void update();
  void multiplex();
  /* Function pointer object instance to call multiplex() from the static interrupt
   * service routine declared as void ftm1_isr(void) */
  static OctadecaTLC5940* me;
  // Start timers and interrupts
  void begin();
private:
  void setChannel(uint16_t channel, uint16_t color);
  void setChannelBuffer(int layer);
  void sendChannelBuffer();
};
#endif
