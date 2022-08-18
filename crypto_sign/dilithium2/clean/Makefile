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

LIB=libdilithium2_clean.a
HEADERS=api.h ntt.h packing.h params.h poly.h polyvec.h reduce.h rounding.h sign.h symmetric.h 
OBJECTS=ntt.o packing.o poly.o polyvec.o reduce.o rounding.o sign.o symmetric-shake.o 

# CFLAGS=-O3 -Wall -Wextra -Wpedantic -Werror -Wmissing-prototypes -Wredundant-decls -std=c99 -I../../../common $(EXTRAFLAGS)
CFLAGS=-g -DRV_ISA=$(ISA_STR) -I../../../common $(EXTRAFLAGS)

all: $(LIB)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(AR) -r $@ $(OBJECTS)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIB)
