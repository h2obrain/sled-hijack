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
#include <drivers/otm8009a_impl.h>
#include <gfx/gfx.h>
#include <ip/dma2d.h>
#include "../sled/src/matrix.h"
#include "../sled/src/random.h"
#include "../sled/src/timers.h"
#include <limits.h>

#include "../sled/src/taskpool.h"
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
		taskpool_numbers = realloc(taskpool_numbers, end * sizeof(int));;
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


#define PX_SIZE 10
static inline
int cs(int size) {
//	return ((size/PX_SIZE)-1) & ~0x1; // i'm quite sure some modules have special requirements for the size (eg. to be an equal number)
	return size / PX_SIZE; // - 1;
}

#define X_OFF 10
#define Y_OFF 70

#define ABSI(i) (i<0?-i:i)
#define MAXI(a,b) (a>b?a:b)
#define MINI(a,b) (a>b?b:a)

static dma2d_pixel_buffer_t *pxdst = NULL;
static dma2d_pixel_buffer_t _pxdst;
static dma2d_pixel_buffer_t *pxsrc = NULL;
static dma2d_pixel_buffer_t _pxsrc;
static uint8_t _pxsrc_buffer[PX_SIZE][PX_SIZE];

// This is where the matrix functions send output.
// It is the root of the output chain.
//static int mod_out_no;
int matrix_init(int outmodno) {
	(void)outmodno;
	printf("matrix_init(%d)\n",outmodno);

	if (pxdst == NULL) {
		printf("initializing pxdst w:%d h:%d\n",matrix_getx(),matrix_gety());
		dma2d_setup_ltdc_pixel_buffer(DISPLAY_LAYER_2, &_pxdst);
		pxdst = &_pxdst;
	}
	if (pxsrc == NULL) {
		_pxsrc.buffer = _pxsrc_buffer;
		_pxsrc.width  =
		_pxsrc.height = PX_SIZE;
		_pxsrc.in.pixel.bitsize = 8;
		_pxsrc.in.pixel.format  = DMA2D_xPFCCR_CM_A8;
		_pxsrc.in.pixel.alpha_mode.color = 0;
		_pxsrc.out.pixel.bytesize = // out is not supported!
		_pxsrc.out.pixel.format = 0;
	//	double max_v = PX_SIZE * pow(2,0.5)/2;
		for (int16_t y=0;y<PX_SIZE; y++) {
			for (int16_t x=0;x<PX_SIZE; x++) {
				double xd  = (double)x-(double)PX_SIZE/2;
				double yd  = (double)y-(double)PX_SIZE/2;
				double d   = 255*pow(pow(xd*xd+yd*yd,0.5)/(PX_SIZE/2),2); //max_v;
				uint32_t a = (uint32_t)lround(d);
				_pxsrc_buffer[y][x] = 255-(uint8_t)MINI(a,255); // = 255*ABSI((x+y)/2+1-PX_SIZE)/PX_SIZE;
//				((uint8_t (*)[PX_SIZE])_pxsrc.buffer)[y][x] = 255-(uint8_t)MINI(a,255); // = 255*ABSI((x+y)/2+1-PX_SIZE)/PX_SIZE;
			}
		}
		pxsrc = & _pxsrc;
	}
	return (pxdst == NULL) || (pxsrc == NULL);
}


int matrix_getx() {
	return cs(gfx_width() -X_OFF-10);
}
int matrix_gety() {
	return cs(gfx_height()-Y_OFF-10);
}

static inline
gfx_color_t RGB_to_argb8888(RGB color) {
	return (gfx_color_t){ .argb8888 = { .a=color.alpha, .r=color.red, .g=color.green, .b=color.blue} };
}
static inline
RGB RGB_from_argb8888(gfx_color_t c) {
	return (RGB){ .alpha=c.argb8888.a, .red=c.argb8888.r, .green=c.argb8888.g, .blue=c.argb8888.b };
}

int matrix_set(int x, int y, RGB color) {
	assert((x<matrix_getx()) && (y<matrix_gety()));
//	gfx_draw_pixel((int16_t)x,(int16_t)y,RGB_to_arPX_SIZEgb8888(color));
//	init_pxdst();
//	dma2d_fill(pxdst, RGB_to_argb8888(color).raw, X_OFF+x*PX_SIZE,Y_OFFy*PX_SIZE, PX_SIZE,PX_SIZE);
//	init_pxsrc();
	pxsrc->in.pixel.alpha_mode.color = RGB_to_argb8888(color).raw;
	dma2d_convert_copy__no_pxsrc_fix(pxsrc,pxdst, 0,0, X_OFF+x*PX_SIZE,Y_OFF+y*PX_SIZE, PX_SIZE,PX_SIZE);
	return 0;
}

RGB matrix_get(int x, int y) {
	assert((x<matrix_getx()) && (y<matrix_gety()));
	return RGB_from_argb8888(gfx_get_pixel(X_OFF+(int16_t)x*PX_SIZE+PX_SIZE/2,Y_OFF+(int16_t)y*PX_SIZE+PX_SIZE/2));
}

// Fills part of the matrix with jo-- a single color.
int matrix_fill(int start_x, int start_y, int end_x, int end_y, RGB color) {
	if (start_x > end_x)
		return 1;
	if (start_y > end_y)
		return 2;

	start_x = X_OFF + start_x * PX_SIZE;
	end_x   = X_OFF + end_x   * PX_SIZE;
	start_y = Y_OFF + start_y * PX_SIZE;
	end_y   = Y_OFF + end_y   * PX_SIZE;

//	init_pxdst();
	dma2d_fill(
			pxdst,
			RGB_to_argb8888(color).raw,
			start_x, start_y,
			end_x-start_x,end_y-start_y
		);

//	int x;
//	int y;
//	for (y = MAX(start_y, 0); y <= MIN(end_y, matrix_gety()); y++)
//		for (x = MAX(start_x, 0); x <= MIN(end_x, matrix_getx()); x++) {
//			matrix_set(X_OFF+x, Y_OFF+y, color);
//		}
	return 0;
}

// Zeroes the stuff.
int matrix_clear(void) {
//	init_pxdst();
	dma2d_fill(pxdst,0,X_OFF,Y_OFF,matrix_getx()*PX_SIZE,matrix_gety()*PX_SIZE);
	return 0;
}

static int current_mod = -1;
uint32_t update_led_counter(void);
int matrix_render(void) {
	while (!display_ready());

	uint64_t time = mtime();
#define FPS_SAMPLE_TIME 1000
	static double fps = 0;
	static uint64_t fps_timeout = FPS_SAMPLE_TIME;
	static uint32_t c = 0;
	static char buf[256]={0};
	c++;
	if (fps_timeout<=time) {
		fps = (double)(1000*c)/(time+FPS_SAMPLE_TIME-fps_timeout);
		c = 0;
		fps_timeout = time+FPS_SAMPLE_TIME;
		sprintf(buf, "% 7.03f fps", fps);
	}
	//gfx_set_font_scale(2);
//#define font font_Tamsyn10x20b_20
#define font (&font_DejaVuSansMono_36)
//	init_pxdst();
//	gfx_fill_rect(0,0, gfx_width(),10+font->lineheight, (gfx_color_t){.argb8888={.c=0xff888888}});
	dma2d_fill(pxdst, 0xff000000, 0,0, gfx_width(),10+font->lineheight);
//	gfx_set_font_blending(false);
	gfx_puts2(10,10,buf,font,(gfx_color_t){.argb8888.c=0xffffffff});
	gfx_puts("  ");
	if (current_mod>=0) {
		gfx_puts(sled_modules[current_mod]->name);
	} else {
		gfx_puts("no mod!");
	}

	dma2d_wait_complete();

	display_update();

	/* wait here to avoid taking precious bandwidth from ltdc/dsi */
	while (!display_ready());

	update_led_counter();

	return 0;
}

int matrix_deinit(void) {
	//~ ..
	return 0;
}

/* move somewhere.. */
static int next_mod=-1;
void matrix_next_animation() {
	if (current_mod>=0) {
		next_mod = (current_mod+1) % SLED_MODULE_COUNT;
	} else {
		next_mod = 0;
	}
}

int matrix_pick_next(ulong delay_us) {
	if (current_mod>=0) sled_modules[current_mod]->deinit(current_mod);
	if (delay_us==ULONG_MAX) return 0;
	current_mod = rand()%SLED_MODULE_COUNT;
	sled_modules[current_mod]->init(current_mod);
	sled_modules[current_mod]->reset(current_mod);
	if (!delay_us)  delay_us++;
	return timer_add(delay_us, current_mod, 0, NULL);
}

void matrix_run_main_loop() {
	timers_init(0);
	matrix_init(0);
	//	for (uint32_t i=0; i<SLED_MODULE_COUNT; i++) {
	//		sled_modules[i]->init(i);
	//	}
	matrix_pick_next(0);
	while (1) {
		if (next_mod!=-1) {
			timer_add(0,-1,0,NULL); timer_get(); /* clear timers */
			current_mod = next_mod; next_mod = -1;
			timer_add(udate()+10000000, current_mod, 0, NULL);
		}
		timer tnext = timer_get();
		if (tnext.moduleno == -1) {
			// Queue random.
			matrix_pick_next(udate()+10000000);
		} else
		if (tnext.time > udate()) {
			// Early break. Set this timer up for elimination by any 0-time timers that have come along
			//if (tnext.time == 0) tnext.time = 1;
			timer_add(tnext.time, tnext.moduleno, 0,NULL);
			continue;
		} else {
			int r = sled_modules[tnext.moduleno]->draw(tnext.moduleno);
			switch (r) {
				case 0:
					break;
				case 1:
					matrix_pick_next(udate()+1000000);
					break;
				default :
					for (uint32_t i=0; i<SLED_MODULE_COUNT; i++) {
						sled_modules[i]->deinit(i);
					}
					while (1);
					break;
			}
		}
	}
}
