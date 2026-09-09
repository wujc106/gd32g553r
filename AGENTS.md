# GD32 DevSuite for VS Code project guidance

This repository is configured for GD32 Cortex-M devices and can be reused as a template for other GD32-based projects.

## Project intent

- Build with CMake + Ninja
- Use the GD32DevSuite for VS Code plugin for project management and debugging
- Compile with GNU ARM cross-toolchain
- Download and debug through GD-Link/OpenOCD-compatible configuration

## Required project layout

```text
project-root/
├─ .project.json                  # plugin project definition and debug config
├─ CMakeLists.txt                 # main CMake project entry
├─ CMakePresets.json              # Debug/Release presets
├─ arm-none-eabi-gcc.cmake        # cross toolchain config
├─ ldscripts/                     # linker scripts
│  └─ gd32g5x3_flash.ld
├─ gcc_startup/                   # startup assembly files
│  └─ startup_gd32g5x3.S
├─ Firmware/                      # GD32 CMSIS + peripheral drivers
│  ├─ CMSIS/
│  └─ GD32G5x3_standard_peripheral/
├─ inc/                           # application headers
├─ src/                           # application source files
├─ build/                         # generated build output
├─ openocd_gdlink/                # GD-Link/OpenOCD debug cfg
│  └─ openocd_gdlink.cfg
├─ lvgl/                          # optional GUI library source
└─ lv_conf.h                     # optional LVGL config
```

## Core configuration rules

### 1. MCU definition

Use a compile definition matching the target chip. Example:

```cmake
add_compile_definitions(
    GD32G553
    GD_ECLIPSE_GCC
    USE_STDPERIPH_DRIVER
)
```

For other projects, replace `GD32G553` with the actual MCU, for example:

- `GD32F303`
- `GD32E230`
- `GD32G5x3`
- `GD32L233`

### 2. Toolchain

The toolchain file must define the ARM cross-compiler and binutils:

```cmake
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m33)
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(CMAKE_SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
```

### 3. Linker script and startup

Every project must provide:

- A linker script in `ldscripts/`
- A reset/interrupt startup file in `gcc_startup/`
- A correct `ENTRY(Reset_Handler)` in the linker script

### 4. Include paths

Add all SDK include roots:

```cmake
set(INC_DIRS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32G5x3/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32G5x3_standard_peripheral/Include
    ${CMAKE_SOURCE_DIR}/inc
)
```

### 5. Source discovery

For ordinary projects, keep a simple and explicit source layout:

```cmake
file(GLOB_RECURSE SOURCE_GLOB_0 CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE SOURCE_GLOB_1 CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/gcc_startup/*.c" "${CMAKE_SOURCE_DIR}/gcc_startup/*.s" "${CMAKE_SOURCE_DIR}/gcc_startup/*.S")
file(GLOB_RECURSE SOURCE_GLOB_2 CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Firmware/GD32G5x3_standard_peripheral/Source/*.c")
file(GLOB_RECURSE SOURCE_GLOB_3 CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32G5x3/Source/*.c")
```

Then combine them into one executable target.

## GD32DevSuite for VS Code configuration

The plugin uses `.project.json` to describe the project and debug configuration.

### Example important fields

```json
{
  "projectname": "oled_test",
  "mcu": "GD32G553RE",
  "debug": {
    "activeConfiguration": "Debug",
    "configurations": [
      {
        "name": "Debug",
        "probe": "gdlink",
        "sn": "",
        "cfg": [
          "E:/GD32/user_code/oled_test/openocd_gdlink/openocd_gdlink.cfg"
        ],
        "interface": "swd",
        "speed": 4000,
        "svdFile": "GD32G5x3.svd",
        "elfFile": "build/Debug/oled_test.elf",
        "resetOnConnect": true,
        "loadSymbolsOnly": false,
        "freezePeripherals": false
      }
    ]
  },
  "ProjectActiveConfigurations": "Debug"
}
```

For other projects, update:

- `projectname`
- `mcu`
- `cfg` path to your OpenOCD/GD-Link script
- `svdFile` to match the target SVD
- `elfFile` to the active preset output path
- `interface`: usually `swd`
- `speed`: adjust as needed

The `configurations` section must contain complete Debug and Release build
definitions (targetProcessor, optimization, warnings, debugging, outputFile,
compiler, linker, assembler). An incomplete `configurations` block causes the
plugin to fail to initialize the debug session even when `probe` and `cfg` are
correct.

## Debug configuration

Use a project-local OpenOCD script for the target setup. The `cfg` field in
`.project.json` must point to this file for both GD-Link and OpenOCD sessions.

```text
openocd_gdlink/openocd_gdlink.cfg
```

The script must match the MCU family and the GD-Link adapter. Typical entries:

```cfg
adapter driver cmsis-dap
source [find target/swj-dp.tcl]
set _CHIPNAME gd32g5x3
set _CPUTAPID 0x0BE12477
adapter speed 1000
reset_config srst_nogate
```

## Build process

This repository follows the standard GD32DevSuite flow:

1. Configure toolchain via `arm-none-eabi-gcc.cmake`
2. Generate Ninja files with `CMakePresets.json`
3. Build in chosen preset: `Debug` or `Release`
4. Convert output to `.hex`, `.bin`, `.lst`
5. Start debug from the plugin

### Example preset structure

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "base",
      "hidden": true,
      "generator": "Ninja",
      "binaryDir": "${sourceDir}/build/${presetName}",
      "toolchainFile": "${sourceDir}/arm-none-eabi-gcc.cmake",
      "cacheVariables": {
        "CMAKE_EXPORT_COMPILE_COMMANDS": "ON"
      }
    },
    {
      "name": "Debug",
      "inherits": "base",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ]
}
```

## Typical build commands

```bash
cmake --preset Debug
cmake --build --preset Debug
```

## Post-build output

The build emits:

- `project_name.elf`
- `project_name.hex`
- `project_name.bin`
- `project_name.map`
- `project_name.lst`

This is handled by post-build commands in `CMakeLists.txt`.

## Migration checklist for other projects

When adapting a normal project to this workflow, do the following:

1. Replace the MCU type in `CMakeLists.txt` and `.project.json`
2. Confirm the correct ARM family, core, and startup file
3. Add the vendor CMSIS include paths and peripheral include paths
4. Add the linker script and startup file
5. Update the debug script path in `.project.json`
6. Check the target clock and reset configuration
7. Validate `SWD`/`J-Link` or `GD-Link` connectivity
8. Build with CMake preset and verify generated firmware files

## Common pitfalls

- Wrong `mcu` name in `.project.json`
- Wrong target core in the plugin config
- Missing startup file or incorrect linker script
- Include path mismatch for CMSIS or peripheral headers
- Debug script refers to wrong chip family or adapter type
- Using stale J-Link / GD-Link firmware
- Incomplete `configurations` block (missing Debug Release, optimization, warnings, debugging, outputFile, assembler) causes the debug probe to not initialize

## Recommended reuse pattern

For a generic GD32 project, use this repository as the baseline and keep only the MCU-specific files and source tree.

## Notes

This repo currently contains a working GD32G5x3 example with LVGL and CMake-based build flow. It can be reused as a migration template for ordinary MCU firmware projects that need plugin-driven project management and CMake-based compilation.
