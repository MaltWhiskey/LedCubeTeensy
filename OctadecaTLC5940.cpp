#include "OctadecaTLC5940.h"
/* Interrupt Service Routine for FTM1 this is called whenever the BLANK and XLAT combo
 * needs to be pulsed. So at this time we turn off all outputs, and previous clocked
 * in data gets latched in. We must now clock in new data to be latched with the next
 * interrupt. So make sure this interrupt finishes before the next triggers. */
OctadecaTLC5940* OctadecaTLC5940::me;
void ftm1_isr(void) {
  OctadecaTLC5940::me->multiplex();
}
// Initialize all TLC, start the timers and set the static me to enable multiplexing.
OctadecaTLC5940::OctadecaTLC5940() {
  me=this;
}
/* VPRG=GND and DCPRG=VCC in my design, this sets the operating mode in GSPWM mode using
 * the DC-Register, thus never using the EEPROM values. The content of the DC-Register
 * and GS-Register is undefined on starting up, and the VPRG and DCPRG are hardware tied,
 * but we can use a trick to reset the DC-Register by writing 192 bits of dummy data for
 * each TCL5940 and 1 bit extra and than latch it to overflow the input shift register.
 *
 * Note: I assume this trick resets both the DC and GS-Register, but the manual is
 * unclear (8.4.1 Data sheet)
 *
 * First set the BLANK high to prevent turning on any leds when we switch the pin to
 * output mode. */
void OctadecaTLC5940::begin() {
  // Set BLANK high to disable all outputs, there should also be a 10K pull up on this pin.
  digitalWriteFast(BLANK, HIGH);
  pinMode(BLANK, OUTPUT);
  digitalWriteFast(BLANK, HIGH);
  // Set layerpin high to disable the layer. There is a 1K pull-up on each pin.
  for (int i = 0; i < Y_LAYERS; i++) {
    pinMode(m_layerPin[i], OUTPUT);
    digitalWriteFast(m_layerPin[i], HIGH);
  }
  delayMicroseconds(1);
  pinMode(XLAT, OUTPUT);
  pinMode(GSCLK, OUTPUT);
  pinMode(SIN, OUTPUT);
  pinMode(SCLK, OUTPUT);
  digitalWriteFast(XLAT, LOW);
  digitalWriteFast(GSCLK, LOW);
  digitalWriteFast(SIN, LOW);
  digitalWriteFast(SCLK, LOW);
  delayMicroseconds(1);
  
  // Overflow the GS Register and latch this to reset DC-Register and GS-Register.
  for(int i = 0; i < CHNBITS + 1; i++) {
    digitalWriteFast(SCLK, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(SCLK, LOW);
    delayMicroseconds(1);
  }
  digitalWriteFast(XLAT, HIGH);
  delayMicroseconds(1);
  digitalWriteFast(XLAT, LOW);
  delayMicroseconds(1);
  
  // Set All outputs off for the first BLANK, XLAT interrupt
  for(int i = 0; i < CHNBITS; i++) {
    digitalWriteFast(SCLK, HIGH);
    delayMicroseconds(1);
    digitalWriteFast(SCLK, LOW);
    delayMicroseconds(1);
  }

  /* System Clock Gating Control Register 4
   * Compare match timer CMT Clock Gate Control -> Clock enabled */
  SIM_SCGC4 |= SIM_SCGC4_CMT;
  /* CMT Modulator Status and Control Register
   * Disable all modulators and no clock divider prescaler */
  CMT_MSC = 0; 
  // CMT Primary Prescaler Register
  CMT_PPS = 0;
  // Primary Carrier High Time Data Value
  CMT_CGH1 = CGH1;
  // Primary Carrier Low Time Data Value
  CMT_CGL1 = CGL1; 
  // CMT Modulator Data Register Mark High.
  CMT_CMD1 = 0;
  // CMT Modulator Data Register Mark Low
  CMT_CMD2 = 0;
  // CMT Modulator Data Register Space High
  CMT_CMD3 = 0;
  // CMT Modulator Data Register Space Low
  CMT_CMD4 = 0;
  /* CMT Output Control Register 0110 0000
   * CMTPOL (bit 6) active High
   * IROPEN (bit 5) IRO signal is enabled as output  */
  CMT_OC = 0x60;

  // FTM Status and Control clear
  FTM1_SC = 0;
  // Set the start of the counter for the first cycle.
  FTM1_CNTIN = CNTIN;
  // Copies CNTIN to CNT.
  FTM1_CNT = 0;
  // Clear the start of the counter, for all cycles but the first.
  FTM1_CNTIN = 0;
  /* Modulo register contains the modulo. After FTM counter reaches modulo the TOF flag
   * becomes set at next clock Initialize MOD after writing to counters. */
  FTM1_MOD = FTMOD;
  /* Channel 0 Status and Control:
   * MSB:MSA (bit 5:4) channel mode select -> Edge-Aligned PWM
   * ELSB:ELSA (bit 3:2) LOW True pulses */
  FTM1_C0SC = FTM_CSC_MSB|FTM_CSC_ELSA;
  // Channel 1 Status and Control.
  FTM1_C1SC = FTM_CSC_MSB|FTM_CSC_ELSA;
  // Channel 0 Match value
  FTM1_C0V = C0V;
  // Channel 1 Match value
  FTM1_C1V = C1V;

  /* Timer pulses can be tied to specific pins. This enables precise debugging on an
   * Oscilloscope. Pulses can also be generated on any pin in the IRQ, but this will
   * generate a little bit of jitter. The BLANK and XLAT pins are controlled in the IRQ
   * since many more pins need to be controlled during this time */
  // Enable timer to create pulses on GSCLK pin
  GSCLK_PULSE;
  // Disable timer to create pulses on BLANK pin. // BLANK_PULSE;
  BLANK_NOPULSE;
  // Disable timer to create pulses on XLAT pin. // XLAT_PULSE;
  XLAT_NOPULSE;
  /* FTM Status and Control register:
   * Clock source selection -> System clock
   * Center aligned PWM select
   * Enable Interrupt on overflow
   * Select prescaler depending on timer */
  FTM1_SC = FTM_SC_CLKS(1)|FTM_SC_CPWMS|FTM_SC_TOIE|FTM_SC_PS((int)(log(FTMDIV)/log(2)));
  // MCGEN (bit 0) Modulator and Carrier Generator Enabled
  CMT_MSC = 0x01;
  // Set FMT1 Overflow interrupt
  NVIC_ENABLE_IRQ(IRQ_FTM1);
}

/* Sets the next frame ready flag when the display switches from top layer to bottom
 * layer the display and rendering buffers get switched and an empty rendering cube is
 * prepared for a new animation frame. The nextFrameReady is set to false and the
 * animation routines can continue rendering on the empty cube. */
void OctadecaTLC5940::update() {
  m_nextFrameReady = true;
  while(m_nextFrameReady);
}

/* Sets a voxel in the rendering cube so there will be no visual anomalies. */
void OctadecaTLC5940::setVoxel(int x, int y, int z, Color c) {
  m_rgbCube[m_renderingCube][x][y][z] = c;
}

/* Gets a voxel from the displayed cube, so animations can use the current display */
Color OctadecaTLC5940::getDisplayedVoxel(int x, int y, int z) {
  return m_rgbCube[m_displayedCube][x][y][z];
}

/* Gets a voxel from the rendering cube, so animations don't need a buffer */
Color OctadecaTLC5940::getRenderingVoxel(int x, int y, int z) {
  return m_rgbCube[m_renderingCube][x][y][z];
}

// Multiplex only uses digitalWriteFast, this allows the fastest possible timing on
// switching pins. Calculations for next cycle are done in advance after this.
void OctadecaTLC5940::multiplex() {
  // Turn off all outputs before doing anything else
  digitalWriteFast(BLANK, HIGH);
  // Set XLAT to signal new data is available
  digitalWriteFast(XLAT, HIGH);
  // Set currentLayer Pin HIGH to turn off output
  digitalWriteFast(m_currentLayer, HIGH);
  // Set nextLayer Pin LOW to turn on output
  digitalWriteFast(m_nextLayer, LOW);
  // clear XLAT inside the BLANK signal
  digitalWriteFast(XLAT, LOW);
  // Turn on outputs
  digitalWriteFast(BLANK, LOW);
  // Clear interrupt overflow flag register
  if(FTM1_SC & FTM_SC_TOF) {
	 FTM1_SC &= ~FTM_SC_TOF;
  }
  // Determine the next current and next layer
  if (++m_LayerOffset == Y_LAYERS) {
	  m_LayerOffset = 0;
  }
  m_currentLayer = m_layerPin[m_LayerOffset];
  m_nextLayer = m_layerPin[m_LayerOffset+1];
  // Prepare the NEXT layer, this will be send out NEXT multiplex refresh cycle.
  if(m_LayerOffset==Y_LAYERS-1) {
    // If the next animation frame is ready, swap the rendering and displayed cube
    if(m_nextFrameReady) {
	  m_displayedCube = 1-m_displayedCube;
	  m_renderingCube = 1-m_renderingCube;
      // clear rendering canvas for the animation routines to paint on
	  memset(m_rgbCube[m_renderingCube], 0, sizeof(m_rgbCube[0]));
	  // Reset the nextFrameReady flag until a next frame is ready.
      m_nextFrameReady = false;
    }
    setChannelBuffer(0);
  } else {
    setChannelBuffer(m_LayerOffset + 1);
  }
  // Send out the color buffer, using DMA, so this takes no processor time.
  sendChannelBuffer();
}

// Prepares the color buffer to be send to the TLC's
void OctadecaTLC5940::setChannelBuffer(int y) {
  uint16_t *chn;
  for (int x = 0; x < X_LAYERS; x++) {
    for (int z = 0; z < Z_LAYERS; z++) {
      Color c = m_rgbCube[m_displayedCube][x][y][z];
      // Flip axis of the cube, otherwise (0,0,0) would be at the back of the cube
      chn = m_ledChannel[Z_LAYERS-1-z][x];
      setChannel(*chn++, c.B);
      setChannel(*chn++, c.G);
      setChannel(*chn,   c.R);
    }
  }
}

// Find the right bytes in the channelBuffer and set the right bits to include the color
void OctadecaTLC5940::setChannel(uint16_t channel, uint16_t color) {
  uint8_t *index12p = m_channelBuffer + (((CHANNELS - channel - 1) * 3) >> 1);
  if (channel & 0x01) {
    *(index12p++) = color >> 4;
    *index12p = ((uint8_t)(color << 4)) | (*index12p & 0xF);
  } else {
    *index12p = (*index12p & 0xF0) | (color >> 8);
    *(++index12p) = color & 0xFF;
  }
}

/* Sends the channel buffer for the next layer to be displayed */
void OctadecaTLC5940::sendChannelBuffer() {
  SPI.begin();
  // Setup SPI for DMA transfer
  SPI0_SR = 0xFF0F0000;
  SPI0_RSER = 0x00;
  // Make sure SPI triggers a DMA transfer after each transmit
  SPI0_RSER = SPI_RSER_TFFF_RE | SPI_RSER_TFFF_DIRS;
  SPI.beginTransaction(m_spiSettings);
  m_dmaChannel.sourceBuffer(m_channelBuffer, CHNBYTES);
  // Move data into the SPI FIFO register
  m_dmaChannel.destination((volatile uint8_t&)SPI0_PUSHR);
  // Only transfer data once the previous byte has been transmitted
  // This is to ensure all bytes are sent.
  m_dmaChannel.triggerAtHardwareEvent(DMAMUX_SOURCE_SPI0_TX);
  // Stop after transmitting all 256 bytes
  m_dmaChannel.disableOnCompletion();
  m_dmaChannel.enable();
  SPI.endTransaction();
}
