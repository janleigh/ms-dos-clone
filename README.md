# ms-dos-clone
A simple MS DOS clone for my final project.

## Features

- Command-line interface similar to MS-DOS
- Basic file system with directories and files
- Commands for file management (copy, move, delete, etc.)
- Tab completion for file names
- Command history navigation

## Building from Source

### Prerequisites

You'll need the following tools installed:
- GCC with multilib support
- NASM assembler
- GNU Make
- GRUB

### Build Instructions

1. Clone the repository:
```sh
git clone https://github.com/janleigh/ms-dos-clone.git cd ms-dos-clone
```

2. Build the ISO image:
```sh
make && make iso
```
## Running

### Using QEMU (Recommended)

If you have QEMU installed, run:
```sh
make run
```

### Using VirtualBox or VMware

1. Create a new virtual machine
2. Set the OS type to "Other/Unknown"
3. Choose "Use an existing virtual hard disk file" and select the generated ISO file
4. Start the virtual machine