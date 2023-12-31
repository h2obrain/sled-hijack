// The things actually doing the matrix manipulation.
// Also contains the buffers.
//
// Copyright (c) 2019, Adrian "vifino" Pistol <vifino@tty.sh>
// 
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "types.h"
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "sled.h"
#include "matrix.h"
#include "../sled/src/matrix.h"
#include "../sled/src/random.h"
#include "../sled/src/timers.h"
#include <limits.h>

#include "../sled/src/taskpool.h"

int hoi(uint32_t d) {
	int mx = matrix_getx();
	int my = 0; //matrix_gety();
	printf("okay (%d) %d/%d\n", d, mx, my);
	return 170;
}
module_t dummy_module = {
	.name = "Dummy",
	.init = hoi,
};

/* dummy mutex */
oscore_mutex oscore_mutex_new(void) {
	return NULL;
}
void oscore_mutex_lock(oscore_mutex m) {
	(void)m;
}
void oscore_mutex_unlock(oscore_mutex m) {
	(void)m;
}
void oscore_mutex_free(oscore_mutex m) {
	(void)m;
}
/* tasks */
static int* taskpool_numbers;
static int taskpool_numbers_maxn = 0;
void taskpool_forloop(taskpool* pool, void (*func)(void*), int start, int end) {
	(void)pool;
	int s = MAX(start, 0);
	int c = (end) - s;

	if (end > taskpool_numbers_maxn) {
		taskpool_numbers = realloc(taskpool_numbers, end * sizeof(int));
		assert(taskpool_numbers);

		for (int i = taskpool_numbers_maxn; i < end; i++)
			taskpool_numbers[i] = i;

		taskpool_numbers_maxn = end;
	}

	taskpool_submit_array(pool, c, func, &taskpool_numbers[s], sizeof(int));
}
// Hellish stuff to run stuff in parallel.
inline void taskpool_submit_array(taskpool* pool, int count, void (*func)(void*), void* ctx, size_t size) {
	(void)pool;
	for (int i = 0; i < count; i++)
		taskpool_submit(pool, func, ctx + (i * size));
}
int taskpool_submit(taskpool* pool, void (*func)(void*), void* ctx) {
	(void)pool;
//	if (pool->workers <= 1) {
		// We're faking. This isn't a real taskpool.
		func(ctx);
		return 0;
//	}
//	taskpool_job job = {
//		.func = func,
//		.ctx = ctx,
//	};
//	tp_putjob(pool, job);
//	return 0;
}
void taskpool_wait(taskpool* pool) {
	(void)pool;
}


// This is where the matrix functions send output.
// It is the root of the output chain.
//static int mod_out_no;
__attribute__((weak))
int matrix_init(int outmodno) {
	(void)outmodno;
	printf("matrix_init(%d)\n",outmodno);
	return 0;
}

__attribute__((weak))
int matrix_getx() {
	return 0;
}
__attribute__((weak))
int matrix_gety() {
	return 0;
}

__attribute__((weak))
int matrix_set(int x, int y, RGB color) {
	assert((x<matrix_getx()) && (y<matrix_gety()));
	return 0;
}

__attribute__((weak))
RGB matrix_get(int x, int y) {
	assert((x<matrix_getx()) && (y<matrix_gety()));
	return (RGB){0};
}

// Fills part of the matrix with jo-- a single color.
__attribute__((weak))
int matrix_fill(int start_x, int start_y, int end_x, int end_y, RGB color) {
	if (start_x > end_x) return 1;
	if (start_y > end_y) return 2;

	return 0;
}

// Zeroes the stuff.
__attribute__((weak))
int matrix_clear(void) {
	return 0;
}

__attribute__((weak))
int matrix_render(void) {
	return -1;
}

__attribute__((weak))
int matrix_deinit(void) {
	return 0;
}




static int current_mod = -1;

__attribute__((weak))
int matrix_get_current_mod(void) {
	return current_mod;
}

//__attribute__((weak))
void matrix_pick_next(void) { //oscore_time delay_us) {
	int next_mod = -1;
	if (current_mod>=0) {
		sled_modules[current_mod]->deinit(current_mod);
	} else {
		current_mod = -1;
	}

	current_mod = (current_mod+1) % SLED_MODULE_COUNT;
//	current_mod = rand() % SLED_MODULE_COUNT;

	sled_modules[current_mod]->init(current_mod);
	sled_modules[current_mod]->reset(current_mod);
}

__attribute__((weak))
int sledhj_init(void) {
	matrix_init(0);
//	for (uint32_t i=0; i<SLED_MODULE_COUNT; i++) {
//		sled_modules[i]->init(i);
//	}
	return 0;
}
__attribute__((weak))
int sledhj_loop(void) {
	if (current_mod < 0) {
		matrix_pick_next();
	}
	if (current_mod < 0) {
		return -1;
	}

	// render or so
	int r = sled_modules[current_mod]->draw(current_mod);
	switch (r) {
		case 0:
			break;
		case 1:
			current_mod = -1;
			break;
		default :
			// error deinit and reinit
			for (uint32_t i=0; i<SLED_MODULE_COUNT; i++) {
				sled_modules[i]->deinit(i);
			}
			sledhj_init();
			break;
	}
	return 0;
}

