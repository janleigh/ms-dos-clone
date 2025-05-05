CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -nostdinc -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c -I./include
LDFLAGS = -T core/linker.ld -melf_i386
ASFLAGS = -f elf32

OBJDIR = ../obj
BINDIR = ../bin

# Core system files
CORE_DIR = core
CORE_SOURCES = $(wildcard $(CORE_DIR)/*.c)
CORE_ASM = $(wildcard $(CORE_DIR)/*.asm)
CORE_OBJECTS = $(patsubst $(CORE_DIR)/%.c, $(OBJDIR)/core/%.o, $(CORE_SOURCES))
CORE_ASM_OBJECTS = $(patsubst $(CORE_DIR)/%.asm, $(OBJDIR)/core/%.o, $(CORE_ASM))

# Device drivers
DRIVERS_DIR = drivers
DRIVERS_SOURCES = $(wildcard $(DRIVERS_DIR)/*.c)
DRIVERS_OBJECTS = $(patsubst $(DRIVERS_DIR)/%.c, $(OBJDIR)/drivers/%.o, $(DRIVERS_SOURCES))

# Shell & commands
SHELL_DIR = shell
SHELL_SOURCES = $(wildcard $(SHELL_DIR)/*.c)
SHELL_OBJECTS = $(patsubst $(SHELL_DIR)/%.c, $(OBJDIR)/shell/%.o, $(SHELL_SOURCES))

# Utilities
UTILS_DIR = utils
UTILS_SOURCES = $(wildcard $(UTILS_DIR)/*.c)
UTILS_OBJECTS = $(patsubst $(UTILS_DIR)/%.c, $(OBJDIR)/utils/%.o, $(UTILS_SOURCES))

# All objects
ALL_OBJECTS = $(CORE_OBJECTS) $(CORE_ASM_OBJECTS) $(DRIVERS_OBJECTS) $(SHELL_OBJECTS) $(UTILS_OBJECTS)

KERNEL = $(BINDIR)/kernel.bin
ISO = ../ms-dos-clone.iso

.PHONY: all clean run iso debug directories

all: directories $(KERNEL)

directories:
	@mkdir -p $(OBJDIR)/core
	@mkdir -p $(OBJDIR)/drivers
	@mkdir -p $(OBJDIR)/shell
	@mkdir -p $(OBJDIR)/utils
	@mkdir -p $(BINDIR)

$(KERNEL): $(ALL_OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJDIR)/core/%.o: $(CORE_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/core/%.o: $(CORE_DIR)/%.asm
	$(AS) $(ASFLAGS) -o $@ $<

$(OBJDIR)/drivers/%.o: $(DRIVERS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/shell/%.o: $(SHELL_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

$(OBJDIR)/utils/%.o: $(UTILS_DIR)/%.c
	$(CC) $(CFLAGS) -o $@ $<

iso: $(KERNEL)
	@mkdir -p ../iso/boot/grub
	@cp $(KERNEL) ../iso/boot/
	@cp core/grub.cfg ../iso/boot/grub/
	@echo "Generating ISO image..."
	@grub-mkrescue -o $(ISO) ../iso 2>/dev/null || \
	grub-mkrescue --xorriso=/usr/bin/xorriso -o $(ISO) ../iso

run: iso
	qemu-system-i386 -cdrom $(ISO)

debug: iso
	qemu-system-i386 -cdrom $(ISO) -s -S &
	gdb -ex "target remote localhost:1234" -ex "symbol-file $(KERNEL)"

clean:
	rm -rf $(OBJDIR) $(BINDIR) ../iso $(ISO)