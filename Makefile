CC ?= gcc

SRC_DIR = ./src
OUT_DIR = ./bin


CFLAGS += -pthread
CFLAGS += -std=c99 
CFLAGS += -Wpedantic
CFLAGS += -Wall
CFLAGS += -Wextra
# https://lists.gnu.org/archive/html/bug-gnulib/2012-09/msg00006.html
# > In the GNU apps I'm familiar with (Emacs, coreutils, ...) we simply disable -Waggregate-return. 
# > It a completely anachronistic warning, since its motivation was to support backwards 
# > compatibility with C compilers that did not allow returning structures. Those compilers are 
# > long dead and are no longer of practical concern.
CFLAGS += -Wno-aggregate-return
CFLAGS += -Wbad-function-cast
CFLAGS += -Wcast-align
CFLAGS += -Wcast-qual
CFLAGS += -Wfloat-equal
CFLAGS += -Wformat=2
CFLAGS += -Wlogical-op
CFLAGS += -Wmissing-include-dirs
CFLAGS += -Wnested-externs
CFLAGS += -Wpointer-arith
CFLAGS += -Wredundant-decls
CFLAGS += -Wsequence-point
CFLAGS += -Wshadow
CFLAGS += -Wswitch
CFLAGS += -Wundef
CFLAGS += -Wunreachable-code
CFLAGS += -Wwrite-strings
CFLAGS += -Wno-discarded-qualifiers

CFILES = $(SRC_DIR)/*.c

all: outdir release debug asan

release: .
	$(CC) -O2 $(CFLAGS) \
	$(CFILES) -o $(OUT_DIR)/$@

debug: .
	$(CC) -O0 -DDEBUG -g $(CFLAGS) \
	$(CFILES) -o $(OUT_DIR)/$@

asan: .
	$(CC) -O0 -g -fsanitize=address -fsanitize=undefined -fsanitize=leak $(CFLAGS) \
	$(CFILES) -o $(OUT_DIR)/$@

outdir:
	mkdir -p $(OUT_DIR)

clean:
	rm -rf $(OUT_DIR)/*

.PHONY: all clean outdir