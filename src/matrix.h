// Helper definitions that will allow one to manipulate the matrix.

#ifndef __INCLUDED_MATRIX__
#define __INCLUDED_MATRIX__

#include "types.h"


// The matrix code is a wrapper for the top output module,
//  though also contains the occasional utility function.
// It does not init or deinit the top output module anymore.
extern int matrix_init(int outmodno);
extern int matrix_getx(void);
extern int matrix_gety(void);
extern int matrix_set(int x, int y, RGB color);
extern RGB matrix_get(int x, int y);
extern int matrix_fill(int start_x, int start_y, int end_x, int end_y, RGB color);
extern int matrix_clear(void);
extern int matrix_render(void);
extern int matrix_deinit(void);

extern void matrix_run_main_loop(void);
extern int matrix_pick_next(ulong delay_us);
extern void matrix_next_animation(void);

#endif

