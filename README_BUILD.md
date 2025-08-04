# Building STM32 Router with CMake

## Prerequisites
- ARM GCC Toolchain (arm-none-eabi-gcc)
- CMake 3.16 or higher
- Make or Ninja build system

## Quick Build

```bash
# Make the build script executable (first time only)
chmod +x build.sh

# Build the project
./build.sh
```

## Manual Build

```bash
# Create build directory
mkdir build
cd build

# Configure (Debug build)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
make -j$(nproc)
```

## Build Outputs
- `STM32_Router.elf` - Debug ELF file
- `STM32_Router.hex` - Intel HEX format for flashing
- `STM32_Router.bin` - Binary format for flashing
- `STM32_Router.map` - Linker map file

## Checking for Errors
The build will fail if there are any compilation errors. Check the output for:
- Syntax errors
- Missing includes
- Undefined symbols
- Type mismatches

## Flashing (with OpenOCD)
```bash
openocd -f interface/stlink.cfg -f target/stm32f7x.cfg -c "program STM32_Router.elf verify reset exit"
```

## Build Configuration
- Target: STM32F745VETx
- CPU: Cortex-M7
- FLASH: 512KB (0x08000000)
- RAM: 256KB (0x20000000)
- FPU: FPv5-D16
- Floating point: hard
