# Variables
SRC = source/main.c \
      source/timer.c \
      source/print.c \
      wiznet/source/system_W7500x.c \
      wiznet/source/W7500x_gpio.c \
      wiznet/source/W7500x_ssp.c \
      wiznet/source/W7500x_uart.c \
      wiznet/source/W7500x_it.c \
      wiznet/source/W7500x_wztoe.c 

ASRC = wiznet/source/startup_W7500.S

INC = -Iinclude/ \
      -Iwiznet/include/ \
      -Iwiznet/include/CMSIS/

# Toolchain
CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
OBJCOPY = arm-none-eabi-objcopy

# Compiler flags
CFLAGS = -mcpu=cortex-m0 -mthumb $(INC) 
ASFLAGS = -mcpu=cortex-m0 -mthumb
LDFLAGS = -mcpu=cortex-m0 -mthumb -nostdlib -T gcc_W7500.ld

ifeq ($(BOARD),SURF5)
	SRC += source/smi.c 
	CFLFAGS += -DSURF5
else
	SRC += source/spi_smi.c 
endif
	
# Output files
NAME = w7500_spi2eth
TARGET =$(NAME).elf
BIN =$(NAME).bin

# Object files
OBJ = $(SRC:.c=.o) $(ASRC:.S=.o)

# Default target
all: $(TARGET) $(BIN)

# Link the object files to create the ELF file
$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile C source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble assembly source files to object files
%.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

# Convert ELF to binary
$(BIN): $(TARGET)
	$(OBJCOPY) -O binary $< $@

# Clean up build files
clean:
	$(RM) $(OBJ) $(TARGET) $(BIN)

