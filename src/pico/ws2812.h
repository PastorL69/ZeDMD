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

#define LEFT_SPEAKER_PIN 24
#define RIGHT_SPEAKER_PIN 22
#define RGBW false
#define NUM_PIXELS 10
#define FREQ 800000
#define BRIGHTNESS 100 // 0-255

class ws2812 {
 public:
  bool init();
  bool deinit();

  void show();

  inline void put_pixel(uint32_t pixel) {
    pio_sm_put_blocking(m_pio, m_stateMachine, RGBW ? pixel : pixel << 8u);
  }

 private:

  PIO m_pio;
  uint m_stateMachine;
  uint m_programOffset;

};

#endif  // WS2812_H
