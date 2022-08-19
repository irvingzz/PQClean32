# This Makefile can be used with GNU Make or BSD Make

# override as desired
TYPE=sign
SCHEME=falcon-512-fp
IMPLEMENTATION=clean

ISA_XLEN  = 32
ISA_BASE  = rv$(ISA_XLEN)ima
ISA_EXT   = 
ISA_STR   = $(ISA_BASE)$(ISA_EXT)
GNU_STRING= riscv$(ISA_XLEN)-unknown-elf
COMPILER_PATH=$(RISCV)
CC        = $(COMPILER_PATH)/bin/$(GNU_STRING)-gcc
CXX       = $(COMPILER_PATH)/bin/$(GNU_STRING)-g++
AS        = $(COMPILER_PATH)/bin/$(GNU_STRING)-as 
LD        = $(COMPILER_PATH)/bin/$(GNU_STRING)-ld 
AR        = $(COMPILER_PATH)/bin/$(GNU_STRING)-ar 
OBJDUMP   = $(COMPILER_PATH)/bin/$(GNU_STRING)-objdump

pk=$(RISCV)/riscv64-unknown-elf/bin/pk
spike=$(RISCV)/bin/spike

SCHEME_DIR=../crypto_$(TYPE)/$(SCHEME)/$(IMPLEMENTATION)

COMMON_DIR=../common
COMMON_FILES=$(COMMON_DIR)/aes.c $(COMMON_DIR)/sha2.c $(COMMON_DIR)/fips202.c $(COMMON_DIR)/sp800-185.c $(COMMON_DIR)/tiny-aes.c $(COMMON_DIR)/performance.c $(COMMON_DIR)/struct64.c
COMMON_HEADERS=$(COMMON_DIR)/*.h
DEST_DIR=../bin_$(SCHEME)_$(IMPLEMENTATION)

SCHEME_LIBRARY=$(SCHEME_DIR)/lib$(SCHEME)_$(IMPLEMENTATION).a
SCHEME_FILES=$(wildcard $(SCHEME_DIR)/*.[chsS])

ELFNAME=$(DEST_DIR)/$(TYPE)_$(SCHEME)_riscv.elf

CFLAGS=-g -DRV_ISA=$(ISA_STR) -I$(COMMON_DIR) $(EXTRAFLAGS)

all: $(DEST_DIR)/$(TYPE)_$(SCHEME)_riscv.elf

$(SCHEME_LIBRARY): $(SCHEME_FILES)
	cd $(SCHEME_DIR) && $(MAKE) COMPILER_PATH=$(COMPILER_PATH)

$(DEST_DIR)/$(TYPE)_$(SCHEME)_riscv.elf: $(SCHEME_LIBRARY) crypto_$(TYPE)/$(TYPE)_$(SCHEME).c $(COMMON_FILES) $(COMMON_DIR)/notrandombytes.c $(COMMON_HEADERS)
	mkdir -p $(DEST_DIR)
	$(CC) $(CFLAGS) -I$(SCHEME_DIR) crypto_$(TYPE)/$(TYPE)_$(SCHEME).c $(COMMON_FILES) $(COMMON_DIR)/notrandombytes.c -o $@ -L$(SCHEME_DIR) -l$(SCHEME)_$(IMPLEMENTATION)
	$(OBJDUMP) -D $(ELFNAME) > $(ELFNAME).dump

run: $(ELFNAME)
	$(spike) --isa=rv32 \
        $(pk) $(ELFNAME) | tee  $(ELFNAME).log

dump: $(ELFNAME)
	$(OBJDUMP) -D $(ELFNAME) > $(ELFNAME).dump

.PHONY: clean
clean:
	$(RM) -r $(DEST_DIR)

.PHONY: cleanall
cleanall: clean
	cd $(SCHEME_DIR) && $(MAKE) clean 

.PHONY: distclean
distclean:
	$(RM) -r $(DEST_DIR)
