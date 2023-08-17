MKF_DIR:=.
include ./sled.mk
include ./sled_rules.mk

default: all

all: lib/libsled.a
default: all

CC =~/.rustup/toolchains/esp/riscv32-esp-elf/esp-12.2.0_20230208/riscv32-esp-elf/bin/riscv32-esp-elf-gcc
ARCH ?=rv32imac_zicsr_zifencei
CFLAGS =-Iinclude -Isled/src -DBUILD_SLED_LIB #-march=$(ARCH)

OBJ=$(addprefix obj/, $(SRCS:.c=.o))

obj/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

lib/libsled.a: $(OBJ)
	mkdir lib
	ar -rcs $@ $^

clean:
	rm -rf obj lib

rust:
	intercept-build --cdb lib/compile_commands.json make clean all
	cd lib; c2rust transpile compile_commands.json
