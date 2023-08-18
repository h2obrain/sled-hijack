MKF_DIR:=.
SPACE:= 
COMMA:=,

include ./sled.mk
include ./sled_rules.mk
SRCS += $(wildcard $(SLED_MK_DIR)/src/*.c)

# $(foreach SRC,$(sort $(SRCS)),$(info $(SRC)))
# $(error okay)


default: all

all: lib/libsled.a
default: all

CC =~/.rustup/toolchains/esp/riscv32-esp-elf/esp-12.2.0_20230208/riscv32-esp-elf/bin/riscv32-esp-elf-gcc
ARCH ?=rv32imac_zicsr_zifencei
CFLAGS =-Iinclude -Isled/src -DBUILD_SLED_LIB -march=$(ARCH) -O3

OBJ=$(addprefix obj/, $(SRCS:.c=.o))

obj/%.o: %.c $(SLED_MK_DIR)/modules/utd
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

lib/libsled.a: $(OBJ)
	mkdir lib
	ar -rcs $@ $^

clean:
	rm -rf obj lib modules

rust:
	intercept-build --cdb lib/compile_commands.json make clean all
	cd lib; c2rust transpile compile_commands.json
