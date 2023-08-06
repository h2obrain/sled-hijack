MKF_DIR:=.
include ./sled.mk

default: all

all: lib/libsled.a

CC=gcc
CFLAGS=-Iinclude -Isled/src -DBUILD_SLED_LIB

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
