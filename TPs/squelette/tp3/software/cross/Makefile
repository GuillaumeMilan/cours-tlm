ifndef CROSS_COMPILE
CROSS_COMPILE=microblaze-xilinx-linux-gnu-
endif

TARGET_CC = $(CROSS_COMPILE)gcc
TARGET_LD = $(CROSS_COMPILE)ld
TARGET_OBJDUMP = $(CROSS_COMPILE)objdump
TARGET_READELF = $(CROSS_COMPILE)readelf
HOST_CC = gcc
HOST_LD = ld
HOST_OBJDUMP = objdump
HOST_READELF = readelf
LDSCRIPT = -T ldscript
INCLUDE = -I. -I../..
BUILD = build
OBJS = $(BUILD)/main.o $(BUILD)/boot.o $(BUILD)/it.o  
EXEC = $(BUILD)/a.out

# Disassembly of the generated executable.
dump.dis: $(EXEC)
	$(TARGET_OBJDUMP) -d $< > $@

# Summary of the sections and their size.
sections.txt: $(EXEC)
	$(TARGET_READELF) -S $< > $@


$(EXEC): $(OBJS)
	$(TARGET_LD) $^ -o $@ $(LDSCRIPT)
# TODO: use $(TARGET_LD), and use the -T option to point to linker script ldscript

$(BUILD)/main.o: ../main.c
	$(TARGET_CC) -c $(INCLUDE) -o $@ $<
# TODO: compile with target C compiler

$(BUILD)/%.o: %.s
	$(TARGET_CC) -c $< -o $@


.PHONY: clean realclean
clean:
	-$(RM) $(OBJS) $(EXEC) dump.dis sections.txt

realclean: clean
	-$(RM) *~
