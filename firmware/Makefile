###############################################################################
#
# Example makefile
#
###############################################################################

### Project name (also used for output file name)
PROJECT = main

### Source files and search directory
CSRC    = main.c 
CSRC   += dhcp_client.c dnslkup.c enc28j60.c ip_arp_udp_tcp.c websrv_help_functions.c
CSRC   += hal_hci.c hal_serial.c hal_timer.c hal_analog.c
# CSRC   += radioPinFunctions.c nrf24.c
# CSRC   += coap.c endpoints.c
ASRC    = xitoa.S
VPATH   = ./tuxlib ./hal ./microcoap ./nrf24l01_plus
DEVICE  = atmega328p
F_OSC   = 8000000
PORT 	= /dev/tty.usbserial-A900cbrd 
# AVRDUDE = avrdude -c arduino -p ${DEVICE} -U flash:w:$(PROJECT).hex -P ${PORT} -B 115200
# RESET   = avrdude -c arduino -p ${DEVICE} -P ${PORT} -B 115200
AVRDUDE = avrdude -c usbtiny -p ${DEVICE} -U flash:w:$(PROJECT).hex -B 1
RESET   = avrdude -c usbtiny -p ${DEVICE}

# avrdude -c usbtiny -p m328p -U lfuse:w:0xff:m -U hfuse:w:0xde:m -U efuse:w:0x05:m
# 32768 - 512  = 32256 => 0x7E00

# avrdude -c usbtiny -p m328p -U lfuse:w:0xff:m -U hfuse:w:0xda:m -U efuse:w:0xfd:m
# 32768 - 2048 = 30720 => 0x7800

# WE ARE USING THE FOLLOWING SETTINGS
# avrdude -c usbtiny -p m328p -U lfuse:w:0xff:m -U hfuse:w:0xd8:m -U efuse:w:0xfd:m
# 32768 - 4096 = 28672 => 0x7000

BOOTSTART = 0x7000

###############################################################################
#
# Don't change anything below
#
###############################################################################

### Include dirs, library dirs and definitions
LIBS	=
LIBDIRS	=
INCDIRS	=
DEFS	= F_CPU=$(F_OSC)
ADEFS	= F_CPU=$(F_OSC)

### Optimization level (0, 1, 2, 3, 4 or s)
OPTIMIZE = s

### C Standard level (c89, gnu89, c99 or gnu99)
CSTD = gnu89

### Warning contorls
WARNINGS = all extra

### Output directory
OBJDIR = obj

### Output file format (ihex, bin or both) and debugger type
OUTPUT	= ihex
DEBUG	= # dwarf-2

### Programs to build porject
CC      = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE    = avr-size
NM      = avr-nm

# Define all object files
COBJ      = $(CSRC:.c=.o) 
AOBJ      = $(ASRC:.S=.o)
COBJ      := $(addprefix $(OBJDIR)/,$(COBJ))
AOBJ      := $(addprefix $(OBJDIR)/,$(AOBJ))
PROJECT   := $(OBJDIR)/$(PROJECT)

CFLAGS         += -Wl,--section-start=.text=$(BOOTSTART) 
CFLAGS          += -Wl,-lm -Wl,--gc-sections,-Map,$(PROJECT).map

# Flags for C files
CFLAGS += -Wno-deprecated-declarations -D__PROG_TYPES_COMPAT__
CFLAGS += -fdata-sections -ffunction-sections 

# http://sourceforge.net/apps/mediawiki/mobilechessboar/index.php?title=Avr-gcc 
# CFLAGS += -mstrict-X -fno-inline-small-functions -fno-move-loop-invariants -fno-optimize-sibling-calls -maccumulate-args
# CFLAGS += -fno-split-wide-types -fno-tree-loop-optimize -fno-tree-switch-conversion -maccumulate-args
# CFLAGS += -fno-dse -fno-caller-saves -fno-tree-ter

CFLAGS += -Wl,--gc-sections
CFLAGS += -std=$(CSTD)
CFLAGS += -g$(DEBUG)
CFLAGS += -mmcu=$(DEVICE)
CFLAGS += -O$(OPTIMIZE) -mcall-prologues
CFLAGS += $(addprefix -W,$(WARNINGS))
CFLAGS += $(addprefix -I,$(INCDIRS))
CFLAGS += $(addprefix -D,$(DEFS))
CFLAGS += -Wp,-M,-MP,-MT,$(OBJDIR)/$(*F).o,-MF,$(OBJDIR)/$(*F).d

# Assembler flags
ASFLAGS += $(addprefix -D,$(ADEFS)) -Wa,-gstabs,-g$(DEBUG)
ALL_ASFLAGS = -mmcu=$(DEVICE) -I. -x assembler-with-cpp $(ASFLAGS) -fdata-sections -ffunction-sections

# Linker flags
LDFLAGS += -Wl,-lm -Wl,--gc-sections,-Map,$(PROJECT).map

# Default target.
all: version build size

ifeq ($(OUTPUT),ihex)
build: elf hex lst sym
hex: $(PROJECT).hex
else
ifeq ($(OUTPUT),binary)
build: elf bin lst sym
bin: $(PROJECT).bin
else
ifeq ($(OUTPUT),both)
build: elf hex bin lst sym
hex: $(PROJECT).hex
bin: $(PROJECT).bin
else
$(error "Invalid format: $(OUTPUT)")
endif
endif
endif

elf: $(PROJECT).elf
lst: $(PROJECT).lst 
sym: $(PROJECT).sym


# Display compiler version information.
version :
	@$(CC) --version

# Create final output file (.hex or .bin) from ELF output file.
%.hex: %.elf
	@echo
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	@echo
	$(OBJCOPY) -O binary $< $@

# Create extended listing file from ELF output file.
%.lst: %.elf
	@echo
	$(OBJDUMP) -h -S -C $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	$(NM) -n $< > $@

# Display size of file.
size:
	@echo
	$(SIZE) -C --mcu=$(DEVICE) $(PROJECT).elf

# Link: create ELF output file from object files.
%.elf:  $(AOBJ) $(COBJ)
	@echo
	@echo Linking...
	$(CC) $(CFLAGS) $(AOBJ) $(COBJ) --output $@

# Compile: create object files from C source files. ARM or Thumb(-2)
$(COBJ) : $(OBJDIR)/%.o : %.c
	@echo
	@echo $< :
	$(CC) -c $(CFLAGS) $< -o $@

# Assemble: create object files from assembler source files. ARM or Thumb(-2)
$(AOBJ) : $(OBJDIR)/%.o : %.S
	@echo
	@echo $< :
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

# Target: clean project.
clean:
	@echo
	rm -f -r $(OBJDIR) | exit 0

# Include the dependency files.
-include $(shell mkdir $(OBJDIR) 2>/dev/null) $(wildcard $(OBJDIR)/*.d)

upload:
	@echo
	$(AVRDUDE) 

reset:
	@echo
	$(RESET)

console:
	@echo
	screen $(PORT) 19200
