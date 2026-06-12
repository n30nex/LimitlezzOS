#ifndef LZ_TICK_H
#define LZ_TICK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Implemented per target: millis() on T-Deck, SDL_GetTicks() in the simulator */
uint32_t lz_tick_ms(void);

#ifdef __cplusplus
}
#endif

#endif
