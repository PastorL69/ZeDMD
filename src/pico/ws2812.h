#ifndef ZEDMD_WS2812_H
#define ZEDMD_WS2812_H

#ifdef PICO_BUILD
#include "pico/zedmd_pico.h"
#endif
#ifdef DMDREADER
#include <dmdreader.h>
#endif

#include "hardware/dma.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "main.h"

#define PARALLEL true
#define PIN_BASE 23
#define PIN_COUNT 2
#define RGBW false
#define NUM_PIXELS 50
#define FREQ 800000 // 800khz speed
#define BRIGHTNESS 15 // 0-255, 25 is already really bright
#define MAX_CYCLES 1000 // 10 seconds of each animation

class ws2812 {
 public:
  bool init();
  bool deinit();

  // updates the ARGB strip
  void show(uint16_t cycles);

  inline void put_pixel(uint32_t pixel) {
    pio_sm_put_blocking(m_pio, m_stateMachine, RGBW ? pixel : pixel << 8u);
  }

 private:

  int t = 0;
  int pat = 0;
  int dir = 1;

  PIO m_pio;
  uint m_stateMachine;
  uint m_programOffset;

};

#endif  // WS2812_H
