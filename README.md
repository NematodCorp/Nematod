# Nematod
A modular NES emulator. Each of its components are if possible, a library that could stand on its own.

## Build  
Nematod uses a CMake build system. Supposing CMake installed, all you have to do is, inside the cloned repository :  
```bash
mkdir build
cd build
cmake ..
make
```
The outputs will be in the bin/ subfolder.

## Run
Usage:
`<executable> your_rom.nes`
