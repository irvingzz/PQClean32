# PQClean32
This repository consists of 321-bit PQClean algorithm , including Kyber, Sphincs, and Dilithium.

## RISCV( on Linux)
### First of all
Change the Makefiles to fix the compiler path and on your own computer, like
    
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
### run
Change directory into
    
    cd PQClean32/test
    vim Makefile
Choose the recipe you want to run
    
    TYPE=sign
    SCHEME=dilithium2
    IMPLEMENTATION=clean
run the peoject
    
    make run
