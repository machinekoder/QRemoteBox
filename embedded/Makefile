################################################################################
# Makefile for the LPC1758 / eStick2 by Martin Horauer
# Modified and improved by Alexander Rössler
#
SOURCEPATH = src
BINPATH = bin
BUILDPATH = build/
SCRIPTPATH = buildscripts
TESTPATH = test

################################################################################
# Sytem files
SRC += $(SOURCEPATH)/mainapplication.c
SRC += $(SOURCEPATH)/storage.c
SRC += $(SOURCEPATH)/remotecommand.c
SRC += $(SOURCEPATH)/remotecontrol.c
SRC += $(SOURCEPATH)/main.c

################################################################################
# Unit tests
TSRC += $(SOURCEPATH)/remotecontrol.test.c

################################################################################
# Libraries
SYSPATH = $(SOURCEPATH)/Libraries/SYS
LIBRARYPATH = $(SOURCEPATH)/Libraries/ComplexCortex/src/lib
#FORMATPATH = $(SOURCEPATH)/Libraries/ComplexCortex/format
XPRINTFPATH = $(SOURCEPATH)/Libraries/ComplexCortex/src/xprintf
DRVPATH = $(SOURCEPATH)/Libraries/ComplexCortex/driver/LPC17xx

INCPATH = -I"$(SYSPATH)"
INCPATH += -I"$(DRVPATH)"
INCPATH += -I"$(SOURCEPATH)/Libraries/ComplexCortex/include"
#INCPATH += -I"$(FORMATPATH)"
#INCPATH += -I"$(FORMATPATH)/lib"

SSRC = $(SYSPATH)/core_cm3.c
SSRC += $(SYSPATH)/NVICInit.c
SSRC += $(SYSPATH)/system_LPC17xx.c
SSRC += $(SYSPATH)/libnosys_gnu.c

#SSRC += $(DRVPATH)/debug_frmwrk.c
#SSRC += $(DRVPATH)/lpc17xx_gpdma.c
#SSRC += $(DRVPATH)/lpc17xx_pinsel.c
#SSRC += $(DRVPATH)/lpc17xx_systick.c
#SSRC += $(DRVPATH)/lpc17xx_adc.c
#SSRC += $(DRVPATH)/lpc17xx_gpio.c
#SSRC += $(DRVPATH)/lpc17xx_pwm.c
#SSRC += $(DRVPATH)/lpc17xx_timer.c
#SSRC += $(DRVPATH)/lpc17xx_can.c
#SSRC += $(DRVPATH)/lpc17xx_i2c.c
#SSRC += $(DRVPATH)/lpc17xx_qei.c
#SSRC += $(DRVPATH)/lpc17xx_uart.c
#SSRC += $(DRVPATH)/lpc17xx_clkpwr.c
#SSRC += $(DRVPATH)/lpc17xx_i2s.c
#SSRC += $(DRVPATH)/lpc17xx_rit.c
#SSRC += $(DRVPATH)/lpc17xx_wdt.c
#SSRC += $(DRVPATH)/lpc17xx_dac.c
#SSRC += $(DRVPATH)/lpc17xx_libcfg_default.c
#SSRC += $(DRVPATH)/lpc17xx_rtc.c
#SSRC += $(DRVPATH)/lpc17xx_emac.c
#SSRC += $(DRVPATH)/lpc17xx_mcpwm.c
#SSRC += $(DRVPATH)/lpc17xx_spi.c
#SSRC += $(DRVPATH)/lpc17xx_exti.c
SSRC += $(DRVPATH)/lpc17xx_nvic.c
#SSRC += $(DRVPATH)/lpc17xx_ssp.c
#SSRC += $(DRVPATH)/ledDriver.c

SSRC += $(LIBRARYPATH)/timer.c
SSRC += $(LIBRARYPATH)/timeout.c
SSRC += $(LIBRARYPATH)/led.c
SSRC += $(LIBRARYPATH)/uart.c
SSRC += $(LIBRARYPATH)/circularbuffer.c
SSRC += $(LIBRARYPATH)/adc.c
SSRC += $(LIBRARYPATH)/dac.c
SSRC += $(LIBRARYPATH)/pwm.c
SSRC += $(LIBRARYPATH)/gpio.c
SSRC += $(LIBRARYPATH)/pincon.c
SSRC += $(LIBRARYPATH)/iap.c
SSRC += $(LIBRARYPATH)/wifly.c
SSRC += $(LIBRARYPATH)/button.c
SSRC += $(LIBRARYPATH)/crc.c
SSRC += $(LIBRARYPATH)/ssp.c
SSRC += $(LIBRARYPATH)/rfm12.c
#SSRC += $(LIBRARYPATH)/debug.c
SSRC += $(LIBRARYPATH)/switch.c
SSRC += $(LIBRARYPATH)/generic.c
SSRC += $(LIBRARYPATH)/spiflash.c
SSRC += $(LIBRARYPATH)/storagemanager.c

SSRC += $(XPRINTFPATH)/xprintf.c

ASRC = $(SYSPATH)/startup_LPC17xx.asm

LDFILE = LPC1758.ld
LDPATH = Linker

################################################################################
# definitions
TARGET = $(BINPATH)/firmware
MCU = cortex-m3
FORMAT = binary

COMPILER_DIR = /home/alexander/bin/arm-toolchain48/bin/
CROSS_COMPILE = $(COMPILER_DIR)arm-none-eabi-
#CROSS_COMPILE = arm-eabi-
AS = $(CROSS_COMPILE)gcc
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
FLASHTOOL = flash_lpc1343.sh

OPT = 0
DEBUG = dwarf-2

OPTFLAGS = -O$(OPT)
CFLAGS += -fmessage-length=0
CFLAGS += -Wall
CFLAGS += -Wa,-adhlns=$(<:%.c=$(BUILDPATH)%.o.lst)
CFLAGS += -g$(DEBUG)
#CFLAGS += -fno-rtti
CFLAGS += -fno-exceptions

ALL_CFLAGS = $(INCPATH) $(CFLAGS) $(OPTFLAGS) -mcpu=$(MCU) -mthumb -g3 
ALL_CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)"

ALL_AFLAGS = -x assembler-with-cpp -Wall -Wa,-adhlns="$(<:%.asm=%.o.lst)"
ALL_AFLAGS += -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" 
ALL_AFLAGS += -mcpu=$(MCU) -mthumb -g3 -g$(DEBUG)

#ifdef SystemRoot
#   RM = del /Q
#   MKDIR = mkdir
#else
#   ifeq ($(shell uname), Linux)
      RM = rm -f
      MKDIR = mkdir -p
#   endif
#endif

################################################################################

all: start $(TARGET).elf $(TARGET).hex $(TARGET).lst $(TARGET).siz end


start:
	@echo "--------------------------------------------------------------------"
	@echo "Starting Build process ..."
	@echo "--------------------------------------------------------------------"
	$(MKDIR) $(BUILDPATH)
	$(MKDIR) $(BINPATH)
end:
	@echo "--------------------------------------------------------------------"
	@echo "End of Build process ..."
	@echo "--------------------------------------------------------------------"

# Compile: create object files from C source files.
.IGNORE: %.asm
$(BUILDPATH)%.o : %.c
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Building file: $(@:%.o=%.c)"
	$(MKDIR) `dirname $@`
	$(CC) -c $(ALL_CFLAGS) -o $@ `echo "$(@:%.o=%.c)" | sed 's,^[^/]*/,,'`
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

# Compile: create object files from ASM source files.
.IGNORE: %.c
$(BUILDPATH)%.o : %.asm
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Building file: $(@:%.o=%.asm)"
	$(MKDIR) `dirname $@`
	$(CC) $(ALL_AFLAGS) -o $@ `echo "$(@:%.o=%.asm)" | sed 's,^[^/]*/,,'`
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""


# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
$(TARGET).elf : $(ASRC:%.asm=$(BUILDPATH)%.o) $(SRC:%.c=$(BUILDPATH)%.o) $(SSRC:%.c=$(BUILDPATH)%.o)
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Linking file ... $<"
	$(LD) -T"./$(LDPATH)/$(LDFILE)" -nostartfiles -L"./$(LDPATH)" $(LD_FLAGS) -Wl,-Map,$(TARGET).map -mcpu=$(MCU) -mthumb -g3 -g$(DEBUG) $^ -o $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""


# Create final output files (.hex, .eep) from ELF output file.
$(TARGET).hex: $(TARGET).elf
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Creating HEX file ... $@"
	$(OBJCOPY) -O $(FORMAT) $< $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

# Create extended listing file from ELF output file.
$(TARGET).lst: $(TARGET).elf
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Creating Listing ... $@"
	$(OBJDUMP) -h -S $< > $@
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

# Create size information
$(TARGET).siz: $(TARGET).elf
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Size output ... $<"
	$(SIZE) --format=berkely -x -t $<
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

test_all: start_test run_test end_test

start_test:
	@echo "--------------------------------------------------------------------"
	@echo "Starting Test process ..."
	@echo "--------------------------------------------------------------------"
	$(MKDIR) $(TESTPATH)
	$(RM) -R $(TESTPATH)/*
end_test:
	@echo "--------------------------------------------------------------------"
	@echo "End of Test process ..."
	@echo "--------------------------------------------------------------------"

# Compile: create object files from C source files.
.IGNORE: %.asm
$(TESTPATH)/%_test : %.test.c
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Building file: $(@:%_test=%.test.c)"
	$(MKDIR) `dirname $@`
	gcc -fprofile-arcs -ftest-coverage $(INCPATH) -lcunit -o $@ `echo "$(@:%_test=%.test.c)" | sed 's,^[^/]*/,,'`
	cp $^ `dirname $@`
	cp $(^:%.test.c=%.c) `dirname $@`
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

run_test : $(TSRC:%.test.c=$(TESTPATH)/%_test)
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
#	cd `dirname $^`
	$^
	mv `basename $(^:%_test=%.test.gcda)` `dirname $^`
	mv `basename $(^:%_test=%.test.gcno)` `dirname $^`
	cd `dirname $^` && lcov --capture --directory ./ --output-file coverage.info
	cd `dirname $^` && genhtml coverage.info --output-directory out
	xdg-open `dirname $^`/out/index.html

# Calculate only the CRC
crc:
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Patching the CRC of the HEX file ..."
	rm -f $(TARGET).bin
	cp $(TARGET).hex $(TARGET).bin
	crc $(TARGET).bin
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

run:
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo "Flashing file to device ..."
	$(SCRIPTPATH)/run.sh
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""

# Program the eStick2
debug:
	@echo ""
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""
	@echo " Starting debug.                                "
	$(SCRIPTPATH)/run_debug.sh
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"

# print some usage message
help:
	@echo "UNIX                                            "
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""
	@echo "To build the project type:                      "
	@echo "make"
	@echo ""
	@echo "To flash the debugger type:"
	@echo "make debug"
	@echo ""
	@echo "To clean the project type:"
	@echo "make clean"
	@echo ""
	@echo "WINDOWS                                         "
	@echo "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~"
	@echo ""
	@echo "To build the project type:                      "
	@echo "cs-make"
	@echo ""
	@echo "To patch the HEX file type:"
	@echo "make crc"
	@echo ""
	@echo "To clean the project type:"
	@echo "make clean"
	@echo ""

.PHONY: all Makefile

clean:
	$(RM) $(TARGET).map $(TARGET).lst
	$(RM) -R $(BUILDPATH)*
