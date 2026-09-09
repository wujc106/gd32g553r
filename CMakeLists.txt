cmake_minimum_required(VERSION 3.20)

project(oled_test C ASM)

set(LINKER_SCRIPT gd32g5x3_flash.ld)

# 编译参数
set(MCU_FLAGS "-mcpu=cortex-m33 -mthumb")

set(CMAKE_C_FLAGS
    "${MCU_FLAGS} -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -std=gnu11"
    CACHE STRING "" FORCE)

set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp" CACHE STRING "" FORCE)

# 各配置编译标志（按 CMAKE_BUILD_TYPE 自动叠加）
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O0 -g" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} \
     -T${LINKER_SCRIPT} \
     -Wl,--print-memory-usage \
     -Xlinker --gc-sections \
     -Wl,-Map=${PROJECT_NAME}.map \
     --specs=nano.specs \
     --specs=nosys.specs \
     -L${CMAKE_SOURCE_DIR}/ldscripts \
     -Wl,--no-warn-rwx-segments"
    CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# 注意：不要加 -ffile-prefix-map=${CMAKE_BINARY_DIR}=${CMAKE_SOURCE_DIR}。
# CMake+Ninja 从 build/<Type>/ 调用 gcc，C 源码 DWARF 形态是
# comp_dir=<ws>/build/<Type> + DW_AT_name=../../src/xxx.c（相对 build 目录）。
# 该 prefix-map 只把 comp_dir 的 build/<Type> 前缀改成 <ws>，却改不到以 ../
# 开头的相对源名，导致 GDB 拼出 <ws>/../../src/xxx.c 再上跳两级，源文件全部找不到。
# 保持默认即可：真实 comp_dir=build/<Type> + ../../src/xxx.c 能正确解析到
# <ws>/src/xxx.c；启动汇编 .S 由调试配置注入的 dir "<ws>/gcc_startup" 兜底。

# 预处理器宏定义
add_compile_definitions(
    GD32G553
    GD_ECLIPSE_GCC
    USE_STDPERIPH_DRIVER
)

# 头文件路径
set(INC_DIRS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32G5x3/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32G5x3_standard_peripheral/Include
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/lvgl
    ${CMAKE_SOURCE_DIR}/lvgl/include
)

set(LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(LV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(LV_BUILD_LVGL_H_SIMPLE_INCLUDE ON CACHE BOOL "" FORCE)
set(LV_BUILD_LVGL_H_SYSTEM_INCLUDE OFF CACHE BOOL "" FORCE)

# 源文件列表：仅保留 MCU 端工程源 + 启动文件 + GD32 外设代码
set(SOURCES)
file(GLOB_RECURSE SOURCE_GLOB_0 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
)
list(REVERSE SOURCE_GLOB_0)
list(APPEND SOURCES ${SOURCE_GLOB_0})

file(GLOB_RECURSE SOURCE_GLOB_1 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/gcc_startup/*.c"
    "${CMAKE_SOURCE_DIR}/gcc_startup/*.s"
    "${CMAKE_SOURCE_DIR}/gcc_startup/*.S"
)
list(REVERSE SOURCE_GLOB_1)
list(APPEND SOURCES ${SOURCE_GLOB_1})

file(GLOB_RECURSE SOURCE_GLOB_2 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/Firmware/GD32G5x3_standard_peripheral/Source/*.c"
)
list(REVERSE SOURCE_GLOB_2)
list(APPEND SOURCES ${SOURCE_GLOB_2})

file(GLOB_RECURSE SOURCE_GLOB_3 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32G5x3/Source/*.c"
)
list(REVERSE SOURCE_GLOB_3)
list(APPEND SOURCES ${SOURCE_GLOB_3})

# 仅编译 LVGL 核心库，不编译 linux-specific examples/tests
add_subdirectory(${CMAKE_SOURCE_DIR}/lvgl ${CMAKE_BINARY_DIR}/lvgl_build)

set(TARGET_ELF ${PROJECT_NAME}.elf)

add_executable(${TARGET_ELF}
    ${SOURCES}
)

target_link_libraries(${TARGET_ELF} PRIVATE lvgl)
target_include_directories(${TARGET_ELF} PRIVATE ${INC_DIRS})

# 构建后处理：默认生成 hex / bin / lst / size；
set(TARGET_HEX ${PROJECT_NAME}.hex)
set(TARGET_BIN ${PROJECT_NAME}.bin)
set(TARGET_LST ${PROJECT_NAME}.lst)

add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${TARGET_ELF}> ${TARGET_HEX}
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${TARGET_ELF}> ${TARGET_BIN}
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide
            $<TARGET_FILE:${TARGET_ELF}> > ${TARGET_LST}
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${TARGET_ELF}>
    COMMENT "Generating ${TARGET_HEX}, ${TARGET_BIN}, ${TARGET_LST}"
)
