// Helper definitions that will allow one to manipulate the matrix.

#pragma once

#include "../sled/src/types.h"


// The matrix code is a wrapper for the top output module,
//  though also contains the occasional utility function.
// It does not init or deinit the top output module anymore.


/* matrix stuff */
#include <gfx/gfx/gfx_argb8888.h>
static inline
gfx_color_t RGB_to_argb8888(RGB col) {
	return (gfx_color_t){ .argb8888 = { .a=col.alpha, .r=col.red, .g=col.green, .b=col.blue} };
}
static inline
RGB RGB_from_argb8888(gfx_color_t c) {
	return (RGB){ .alpha=c.argb8888.a, .red=c.argb8888.r, .green=c.argb8888.g, .blue=c.argb8888.b };
}

int matrix_init(int outmodno);
int matrix_getx(void);
int matrix_gety(void);
int matrix_set(int x, int y, RGB color);
RGB matrix_get(int x, int y);
int matrix_fill(int start_x, int start_y, int end_x, int end_y, RGB color);
int matrix_clear(void);
int matrix_render(void);
int matrix_deinit(void);

int matrix_get_current_mod(void);
void matrix_next_animation(void);
int matrix_pick_next(oscore_time delay_us);

int sledhj_init(void);
int sledhj_loop(void);
