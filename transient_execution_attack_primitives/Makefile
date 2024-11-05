# Folders
SRC:=src
INC:=inc
LNK:=link
OBJ:=obj
BIN:=bin
DMP:=dump
DEP:=dep

CC:=riscv64-unknown-elf-gcc
OBJDUMP:=riscv64-unknown-elf-objdump -S
CFLAGS=-mcmodel=medany -l -std=gnu99 -O0 -g -fno-common -fno-builtin-printf -Wall -I$(INC) -Wno-unused-function -Wno-unused-variable -Wno-unused-but-set-variable
LDFLAGS:=-static -nostdlib -nostartfiles -lgcc
DEPFLAGS=-MT $@ -MMD -MP -MF $(DEP)/$*.d
RM=rm -rf

PROGRAMS= T1_primitive T2_primitive T3_primitive
BINS=$(addprefix $(BIN)/,$(addsuffix .riscv,$(PROGRAMS)))
DUMPS=$(addprefix $(DMP)/,$(addsuffix .dump,$(PROGRAMS)))

.PHONY: all
all: $(BINS) $(DUMPS)

dumps: $(DUMPS)

$(OBJ)/%.o: $(SRC)/%.S
	@mkdir -p $(OBJ)
	$(CC) $(CFLAGS) -D__ASSEMBLY__=1 -c $< -o $@

$(OBJ)/%.o: $(SRC)/%.c
	@mkdir -p $(OBJ)
	@mkdir -p $(DEP)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BIN)/%.riscv: $(OBJ)/%.o $(OBJ)/crt.o $(OBJ)/syscalls.o $(OBJ)/stack.o $(LNK)/link.ld
	@mkdir -p $(BIN)
	$(CC) -T $(LNK)/link.ld $(LDFLAGS) $< $(OBJ)/crt.o $(OBJ)/stack.o $(OBJ)/syscalls.o -o $@

$(DMP)/%.dump: $(BIN)/%.riscv
	@mkdir -p $(DMP)
	$(OBJDUMP) -D $< > $@

.PRECIOUS: $(OBJ)/%.o

clean:
	rm -rf $(BIN) $(OBJ) $(DMP) $(DEP)

-include $(addprefix $(DEP)/,$(addsuffix .d,$(PROGRAMS)))
