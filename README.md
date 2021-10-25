# ChCore

This is the repository of ChCore Operating System for CS3601 (2021 Fall). This project aims to implement a modern microkernel operating system in AArch64.

## Environment Setup

* ChCore is developed with QEMU ARM simulator, Ubuntu 20.04. 

## Operations

### Build

* `make` or `make build`: Build the project in `build` directory.

### Emulate

* `make qemu`: Emulate ChCore in QEMU
* Type `Ctrl+A X` to quit QEMU

### Debug

* `make qemu-gdb`: Start a GDB server running ChCore
* `make gdb`: Start a GDB (`gdb-multiarch`) client
* Type `Ctrl+D` to quit GDB

### Grade

* `make grade`: Show your grade of labs in the current branch

## License

* ChCore is provided through [Modern Operating Systems: Principle and Implementation](https://ipads.se.sjtu.edu.cn/mospi/) under [MulanPSL-1.0](https://ocw.mit.edu/terms/).
