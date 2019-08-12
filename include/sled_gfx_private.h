#include <sled.h>

static int init(int moduleno, char* argstr);
static void reset(int _modno);
static int draw(int _modno, int argc, char* argv[]);
static void deinit(int _modno);
