#include <stdint.h>

#pragma once

typedef struct {
	const char *name;
	int  (*init)(uint32_t modno);
	void (*reset)(uint32_t modno);
	int  (*draw)(uint32_t modno);
	void (*deinit)(uint32_t modno);
} module_t;

#include "../modules/sled_modules.h"
#include "../src/matrix.h"
#include "../sled/src/timers.h"

extern module_t dummy_module;
