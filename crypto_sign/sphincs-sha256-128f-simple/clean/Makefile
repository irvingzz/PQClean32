# This Makefile can be used with GNU Make or BSD Make

ISA_XLEN  = 32
ISA_BASE  = rv$(ISA_XLEN)ima
ISA_EXT   = 
ISA_STR   = $(ISA_BASE)$(ISA_EXT)
GNU_STRING= riscv$(ISA_XLEN)-unknown-elf
COMPILER_PATH=$(COMPILER_PATH)
CC        = $(COMPILER_PATH)/bin/$(GNU_STRING)-gcc
CXX       = $(COMPILER_PATH)/bin/$(GNU_STRING)-g++
AS        = $(COMPILER_PATH)/bin/$(GNU_STRING)-as 
LD        = $(COMPILER_PATH)/bin/$(GNU_STRING)-ld 
AR        = $(COMPILER_PATH)/bin/$(GNU_STRING)-ar 
OBJDUMP   = $(COMPILER_PATH)/bin/$(GNU_STRING)-objdump

LIB=libsphincs-sha256-128f-simple_clean.a

HEADERS = params.h address.h wots.h utils.h fors.h api.h hash.h hash_state.h thash.h sha256.h
OBJECTS =          address.o wots.o utils.o fors.o sign.o hash_sha256.o thash_sha256_simple.o sha256.o

# CFLAGS=-O3 -Wall -Wconversion -Wextra -Wpedantic -Wvla -Werror -Wmissing-prototypes -Wredundant-decls -std=c99 -I../../../common $(EXTRAFLAGS)
CFLAGS=-g -DRV_ISA=$(ISA_STR) -I../../../common $(EXTRAFLAGS)

all: $(LIB)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(AR) -r $@ $(OBJECTS)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIB)
