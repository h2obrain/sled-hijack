SLED_MK_DIR :=$(MKF_DIR)

# gfx lib
INCLUDE_DIRS = include sled/src
SOURCES_DIRS = src

#SRCS += $(SLED_MK_DIR)/sled/src/timers.c

SLED_SRCS = mathey.c random.c color.c graphics.c modules/text.c

ifdef SLED_MODS
SLED_MODS_ORIG  = $(filter $(addprefix $(SLED_MK_DIR)/sled/src/modules/,$(SLED_MODS)),$(wildcard $(SLED_MK_DIR)/sled/src/modules/gfx_*c))
else
# do not compile
IGNORED_SLED_MODULES += gfx_candyswarm.c gfx_ip.c gfx_ursuppe2.c gfx_sort1D_algos.c gfx_testmatrix.c
#IGNORED_SLED_MODULES += gfx_autoterminal.c gfx_candyswarm.c gfx_ip.c gfx_ursuppe2.c gfx_sort1D_algos.c gfx_testmatrix.c
#IGNORED_SLED_MODULES += gfx_candyswarm.c gfx_golc.c gfx_ip.c gfx_testmatrix.c gfx_sort1D_algos.c 
# crash with -O3
#IGNORED_SLED_MODULES += gfx_clock.c gfx_error.c gfx_ip.c gfx_text.c
 
IGNORED_SLED_MODULES := $(addprefix $(SLED_MK_DIR)/sled/src/modules/,$(IGNORED_SLED_MODULES))
SLED_MODS_ORIG  = $(filter-out $(IGNORED_SLED_MODULES),$(wildcard $(SLED_MK_DIR)/sled/src/modules/gfx_*c))
endif

SLED_MODS_FILES = $(subst $(SLED_MK_DIR)/sled/src/modules/,,$(SLED_MODS_ORIG))
SLED_MODS_NAMES = $(subst .c,,$(SLED_MODS_FILES))
SLED_MODS_LOCAL = $(addprefix $(SLED_MK_DIR)/modules/,$(SLED_MODS_FILES))

#$(foreach SRC,$(sort $(SLED_MODS_LOCAL)),$(info $(SRC)))
#$(error lol)

#$(error SLED_DIR:$(SLED_MK_DIR))
SLED_MODS_C =$(SLED_MK_DIR)/modules/sled_modules.c
SLED_MODS_H =$(SLED_MK_DIR)/modules/sled_modules.h
SLED_MODS_H_PATHS =../lib/sled_hijack/include/../modules/sled_modules.h
SLED_MODS_H_PATHS+=../lib/sled_hijack/modules/../modules/sled_modules.h

#$(SLED_MODS_LOCAL)
#.DUMMY: $(SLED_MODS_H) $(SLED_MODS_C)
#SAVE_LIST = $(if $(1),$(file >>$(1), $(wordlist 1,4,$(2)))  $(call SAVE_LIST,$(wordlist 5,$(words $1),$1)))

# Build system integration
#DEPS += $(SLED_MODS_H) # created exclusive rule for sled
SRCS += $(SLED_MODS_C) $(SLED_MODS_LOCAL) $(addprefix $(SLED_MK_DIR)/sled/src/,$(SLED_SRCS))
FILES_TO_CLEAN += $(SLED_MODS_H) $(SLED_MODS_C) $(SLED_MODS_LOCAL)
.SECONDARY: $(SLED_MODS_LOCAL)

#$(foreach SRC,$(sort $(SLED_MODS_H)),$(info $(SRC)))
#$(foreach SRC,$(sort $(SLED_MODS_C)),$(info $(SRC)))
#$(foreach SRC,$(sort $(SLED_MODS_LOCAL)),$(info $(SRC)))
#$(error lol)

