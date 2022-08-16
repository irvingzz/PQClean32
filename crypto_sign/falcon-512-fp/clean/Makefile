# This Makefile can be used with GNU Make or BSD Make

ISA_XLEN  = 32
ISA_BASE  = rv$(ISA_XLEN)ima
ISA_EXT   = 
ISA_STR   = $(ISA_BASE)$(ISA_EXT)
GNU_STRING= riscv$(ISA_XLEN)-unknown-elf
COMPILER_PATH=/opt/riscv32-rv32imac
CC        = $(COMPILER_PATH)/bin/$(GNU_STRING)-gcc
CXX       = $(COMPILER_PATH)/bin/$(GNU_STRING)-g++
AS        = $(COMPILER_PATH)/bin/$(GNU_STRING)-as 
LD        = $(COMPILER_PATH)/bin/$(GNU_STRING)-ld 
AR        = $(COMPILER_PATH)/bin/$(GNU_STRING)-ar 
OBJDUMP   = $(COMPILER_PATH)/bin/$(GNU_STRING)-objdump

LIB=libfalcon-512-fp_clean.a

SOURCES = codec.c common.c fft.c inner.c fpr.c keygen.c pqclean.c rng.c sign.c vrfy.c 
OBJECTS = codec.o common.o fft.o inner.o fpr.o keygen.o pqclean.o rng.o sign.o vrfy.o 
HEADERS = api.h fpr.h inner.h

CFLAGS=-Wall -Wconversion -Wextra -Wpedantic -Wvla -Werror -Wmissing-prototypes -Wredundant-decls -std=c99 -I../../../common $(EXTRAFLAGS)

all: $(LIB)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c -o $@ $<

$(LIB): $(OBJECTS)
	$(AR) -r $@ $(OBJECTS)

clean:
	$(RM) $(OBJECTS)
	$(RM) $(LIB)
