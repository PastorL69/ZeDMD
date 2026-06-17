/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "ws2812.h"

#include "main.h"
#include "pico/ws2812.pio.h"

/**
 * NOTE:
 *  Take into consideration if your WS2812 is a RGB or RGBW variant.
 *
 *  If it is RGBW, you need to set RGBW to true and provide 4 bytes per 
 *  pixel (Red, Green, Blue, White) and use urgbw_u32().
 *
 *  If it is RGB, set IS_RGBW to false and provide 3 bytes per pixel (Red,
 *  Green, Blue) and use urgb_u32().
 *
 *  When RGBW is used with urgb_u32(), the White channel will be ignored (off).
 *
 */

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    r = ((uint16_t)r * BRIGHTNESS) >> 8;
    g = ((uint16_t)g * BRIGHTNESS) >> 8;
    b = ((uint16_t)b * BRIGHTNESS) >> 8;
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    r = ((uint16_t)r * BRIGHTNESS) >> 8;
    g = ((uint16_t)g * BRIGHTNESS) >> 8;
    b = ((uint16_t)b * BRIGHTNESS) >> 8;
    w = ((uint16_t)w * BRIGHTNESS) >> 8;

    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | ((uint32_t)(w) << 24) |
            (uint32_t)(b);
}

static void pattern_snakes(ws2812& strip, uint t) {
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        uint x = (i + (t >> 1)) % 64;
        if (x < 10)
            strip.put_pixel(urgb_u32(0xff, 0, 0));
        else if (x >= 15 && x < 25)
            strip.put_pixel(urgb_u32(0, 0xff, 0));
        else if (x >= 30 && x < 40)
            strip.put_pixel(urgb_u32(0, 0, 0xff));
        else
            strip.put_pixel(0);
    }
}

static void pattern_random(ws2812& strip, uint t) {
  if (t % 8) return;
  for (uint i = 0; i < NUM_PIXELS; ++i) {
    uint32_t color = get_rand_32();
    strip.put_pixel(urgbw_u32(color >> 24, color >> 16, color >> 8, color));
  }
}

static void pattern_sparkle(ws2812& strip, uint t) {
    if (t % 8)
        return;
    for (uint i = 0; i < NUM_PIXELS; ++i)
        strip.put_pixel(rand() % 16 ? 0 : urgbw_u32(0xFF, 0xFF, 0xFF, 0xFF));
}

static void pattern_greys(ws2812& strip, uint t) {
    t %= BRIGHTNESS;
    for (uint i = 0; i < NUM_PIXELS; ++i) {
        strip.put_pixel(t * 0x10101);
        if (++t >= BRIGHTNESS) t = 0;
    }
}

typedef void (*pattern)(ws2812& strip, uint t);
const struct {
    pattern pat;
    const char *name;
} pattern_table[] = {
        {pattern_snakes,  "Snakes!"},
        {pattern_random,  "Random data"},
        {pattern_sparkle, "Sparkles"},
        //{pattern_greys,   "Greys"},
};

bool ws2812::deinit() {
  pio_remove_program_and_unclaim_sm(&ws2812_program, m_pio, m_stateMachine,
                                    m_programOffset);
  return true;
}

bool ws2812::init() {
  pio_claim_free_sm_and_add_program_for_gpio_range(
      PARALLEL ? &ws2812_parallel_program : &ws2812_program, &m_pio,
      &m_stateMachine, &m_programOffset, PIN_BASE, PARALLEL ? PIN_COUNT : 1,
      true);

  for (uint i = PIN_BASE; i < PIN_BASE + (PARALLEL ? PIN_COUNT : 1); i++) {
    pio_gpio_init(m_pio, i);
  }

  pio_sm_set_consecutive_pindirs(m_pio, m_stateMachine, PIN_BASE,
                                 PARALLEL ? PIN_COUNT : 1, true);

  pio_sm_config c =
      PARALLEL ? ws2812_parallel_program_get_default_config(m_programOffset)
               : ws2812_program_get_default_config(m_programOffset);
  if (!PARALLEL) {
    sm_config_set_sideset_pins(&c, PIN_BASE);
  }
  sm_config_set_out_shift(&c, false, true, RGBW ? 32 : PARALLEL ? 32 : 24);
  sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);

  // ws2812_T1 + ws2812_T2 + ws2812_T3
  int cycles_per_bit = 10;
  float div = clock_get_hz(clk_sys) / (FREQ * cycles_per_bit);
  sm_config_set_clkdiv(&c, div);

  pio_sm_init(m_pio, m_stateMachine, m_programOffset, &c);
  pio_sm_set_enabled(m_pio, m_stateMachine, true);

  return true;
}

static int get_pat() {
    return rand() % count_of(pattern_table);
}

static int get_dir() {
    return (rand() >> 30) & 1 ? 1 : -1;
}

void ws2812::show(uint16_t cycles) {
  if (cycles == 0) {
    pat = get_pat();
    dir = get_dir();
  }

  pattern_table[pat].pat(*this, t);
  t += dir;
}