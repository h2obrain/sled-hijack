MKF_DIR:=.
SPACE:= 
COMMA:=,

# Modules causing problems with rust linkung.. (probably size or so, TODO find similarities :))
IGNORED_SLED_MODULES += gfx_cube.c gfx_golc.c gfx_maze.c


include ./sled.mk
include ./sled_rules.mk
SRCS += $(wildcard $(SLED_MK_DIR)/src/*.c)

# $(foreach SRC,$(sort $(SRCS)),$(info $(SRC)))
# $(error okay)

.SUFFIXES: # Delete the default suffixes
.SECONDARY:
.PHONY: all

all: lib
lib: lib/libsled.a

#CC =~/.rustup/toolchains/esp/riscv32-esp-elf/esp-12.2.0_20230208/riscv32-esp-elf/bin/riscv32-esp-elf-gcc
CC =$(lastword $(sort $(wildcard ../.embuild/espressif/tools/riscv32-esp-elf/esp-*/riscv32-esp-elf/bin/riscv32-esp-elf-gcc)))
# ARCH ?=rv32imc
# ARCH ?=rv32imac_zicsr_zifencei
# ARCH ?=rv32i2p1_m2p0_a2p1_c2p0_zicsr2p0_zifencei2p0
# CFLAGS =-Iinclude -Isled/src -DBUILD_SLED_LIB -march=$(ARCH) -O3
CFLAGS =-Iinclude -Isled/src -DBUILD_SLED_LIB -O3 -march=rv32imc -ffunction-sections -fdata-sections -mabi=ilp32 -mcmodel=medany --specs=nosys.specs

OBJ=$(addprefix obj/, $(SRCS:.c=.o))

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

lib/libsled.a: $(OBJ)
	mkdir -p lib
	ar -rcs $@ $^

clean:
	rm -rf obj lib modules

rust:
	intercept-build --cdb lib/compile_commands.json make clean all
	cd lib; c2rust transpile compile_commands.json
