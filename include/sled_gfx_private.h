#include <sled.h>

static inline int init(int moduleno, char* argstr);
static inline void reset(int _modno);
static inline int draw(int _modno, int argc, char* argv[]);
static inline void deinit(int _modno);
