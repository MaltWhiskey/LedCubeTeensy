This project is made to control my 9x9x9 RGB led cube.

The hardware and software are of my own design. PCB’s are designed with KiCad and the acrylic case is designed with autocad.

The harware includes 18 TLC5940 and a Teensy 3.5 microcontroller. The led cube is multiplexed in layers and data is send to the TLC's using DMA over SPI.

All teensy specific hardware controlling is contained in 1 class OctadecaTLC5940. By modifying this class or making your own base class any microcontroller can be used to run the led cube.

See it running here: https://www.youtube.com/channel/UCuQumwFU8Kvs-C-DP2EolAw

